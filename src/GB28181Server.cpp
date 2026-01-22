#include "GB28181Server.h"
#include <QTimer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

namespace GB28181 {

GB28181Server::GB28181Server(QObject *parent)
    : QObject(parent)
    , m_exosipContext(nullptr)
    , m_running(false)
    , m_eventThread(nullptr)
    , m_eventTimer(nullptr)
    , m_snCounter(1)
{
}

GB28181Server::~GB28181Server()
{
    stop();
}

bool GB28181Server::start(const GB28181Config &config)
{
    if (m_running) {
        return false;
    }

    m_config = config;

    if (!initEXosip()) {
        return false;
    }

    // 创建事件处理定时器
    m_eventTimer = new QTimer(this);
    connect(m_eventTimer, &QTimer::timeout, this, &GB28181Server::processEvents);
    m_eventTimer->start(50); // 每50ms处理一次事件

    m_running = true;
    qDebug() << "GB28181 Server started on" << m_config.serverIp << ":" << m_config.serverPort;

    return true;
}

void GB28181Server::stop()
{
    if (!m_running) {
        return;
    }

    m_running = false;

    if (m_eventTimer) {
        m_eventTimer->stop();
        delete m_eventTimer;
        m_eventTimer = nullptr;
    }

    cleanupEXosip();

    qDebug() << "GB28181 Server stopped";
}

bool GB28181Server::initEXosip()
{
    m_exosipContext = eXosip_malloc();
    if (!m_exosipContext) {
        emit errorOccurred("Failed to allocate eXosip context");
        return false;
    }

    if (eXosip_init(m_exosipContext) != 0) {
        emit errorOccurred("Failed to initialize eXosip");
        eXosip_quit(m_exosipContext);
        m_exosipContext = nullptr;
        return false;
    }

    // 监听 UDP
    if (eXosip_listen_addr(m_exosipContext, IPPROTO_UDP,
                           m_config.serverIp.toUtf8().constData(),
                           m_config.serverPort, AF_INET, 0) != 0) {
        emit errorOccurred(QString("Failed to listen on %1:%2")
                          .arg(m_config.serverIp).arg(m_config.serverPort));
        eXosip_quit(m_exosipContext);
        m_exosipContext = nullptr;
        return false;
    }

    // 设置用户代理
    eXosip_set_user_agent(m_exosipContext, "GB28181 Client/1.0");

    return true;
}

void GB28181Server::cleanupEXosip()
{
    if (m_exosipContext) {
        eXosip_quit(m_exosipContext);
        m_exosipContext = nullptr;
    }
}

void GB28181Server::processEvents()
{
    if (!m_exosipContext) {
        return;
    }

    eXosip_event_t *event = nullptr;
    while ((event = eXosip_event_wait(m_exosipContext, 0, 0)) != nullptr) {
        eXosip_lock(m_exosipContext);

        switch (event->type) {
        case EXOSIP_REGISTRATION_NEW:
            handleRegister(event);
            break;

        case EXOSIP_MESSAGE_NEW:
            handleMessage(event);
            break;

        case EXOSIP_CALL_ANSWERED:
            handleInviteAnswered(event);
            break;

        case EXOSIP_CALL_CLOSED:
        case EXOSIP_CALL_RELEASED:
            handleCallClosed(event);
            break;

        default:
            break;
        }

        eXosip_unlock(m_exosipContext);
        eXosip_event_free(event);
    }
}

void GB28181Server::handleRegister(eXosip_event_t *event)
{
    if (!event->request) {
        return;
    }

    QString deviceId = extractDeviceId(event->request->from->url->username);

    qDebug() << "Device register request:" << deviceId;

    // 发送 401 要求认证
    osip_www_authenticate_t *auth = nullptr;
    osip_www_authenticate_init(&auth);
    osip_www_authenticate_set_auth_type(auth, osip_strdup("Digest"));
    osip_www_authenticate_set_realm(auth, osip_strdup(m_config.domain.toUtf8().constData()));
    osip_www_authenticate_set_nonce(auth, osip_strdup("nonce123456"));

    eXosip_message_build_answer(m_exosipContext, event->tid, 401, nullptr);
    eXosip_message_send_answer(m_exosipContext, event->tid, 401, nullptr);

    // TODO: 验证认证信息后发送 200 OK
    // 这里简化处理，直接接受注册
    osip_message_t *answer = nullptr;
    eXosip_message_build_answer(m_exosipContext, event->tid, 200, &answer);

    if (answer) {
        // 添加 Expires 头
        char expires[32];
        snprintf(expires, sizeof(expires), "%d", m_config.expires);
        osip_message_set_expires(answer, expires);

        eXosip_message_send_answer(m_exosipContext, event->tid, 200, answer);
    }

    // 创建设备信息
    DeviceInfo device;
    device.deviceId = deviceId;
    device.ip = event->request->from->url->host;
    device.port = atoi(event->request->from->url->port);
    device.status = DeviceStatus::Online;
    device.registerTime = QDateTime::currentDateTime();
    device.lastKeepalive = QDateTime::currentDateTime();

    emit deviceRegistered(device);
}

void GB28181Server::handleMessage(eXosip_event_t *event)
{
    if (!event->request) {
        return;
    }

    QString deviceId = extractDeviceId(event->request->from->url->username);
    QString xml = extractXmlBody(event->request);

    if (xml.isEmpty()) {
        return;
    }

    // 发送 200 OK
    eXosip_message_send_answer(m_exosipContext, event->tid, 200, nullptr);

    // 解析 XML 消息类型
    QXmlStreamReader reader(xml);
    QString cmdType;

    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement() && reader.name() == "CmdType") {
            cmdType = reader.readElementText();
            break;
        }
    }

    qDebug() << "Received MESSAGE from" << deviceId << "CmdType:" << cmdType;

    if (cmdType == "Keepalive") {
        parseKeepalive(deviceId, xml);
    } else if (cmdType == "Catalog") {
        parseCatalog(deviceId, xml);
    } else if (cmdType == "RecordInfo") {
        parseRecordInfo(deviceId, xml);
    }
}

void GB28181Server::handleInviteAnswered(eXosip_event_t *event)
{
    if (!event->response || event->cid < 0) {
        return;
    }

    qDebug() << "INVITE answered, call_id:" << event->cid << "dialog_id:" << event->did;

    // 解析 SDP 获取媒体信息
    sdp_message_t *sdp = eXosip_get_remote_sdp(m_exosipContext, event->did);
    if (!sdp) {
        return;
    }

    SipSession session;
    session.callId = event->cid;
    session.dialogId = event->did;
    session.startTime = QDateTime::currentDateTime();

    // 解析媒体信息
    sdp_connection_t *conn = eXosip_get_video_connection(sdp);
    if (conn) {
        session.mediaInfo.ip = QString::fromUtf8(conn->c_addr);
    }

    sdp_media_t *media = eXosip_get_video_media(sdp);
    if (media) {
        session.mediaInfo.port = atoi(media->m_port);
    }

    // 获取 SSRC
    char *ssrc_attr = sdp_message_a_att_value_get(sdp, 0, "ssrc");
    if (ssrc_attr) {
        session.mediaInfo.ssrc = QString::fromUtf8(ssrc_attr);
    }

    m_sessions[event->cid] = session;

    emit mediaStreamReady(session);
}

void GB28181Server::handleCallClosed(eXosip_event_t *event)
{
    if (event->cid < 0) {
        return;
    }

    qDebug() << "Call closed, call_id:" << event->cid;

    m_sessions.remove(event->cid);
}

void GB28181Server::parseKeepalive(const QString &deviceId, const QString &xml)
{
    emit keepaliveReceived(deviceId);
}

void GB28181Server::parseCatalog(const QString &deviceId, const QString &xml)
{
    QList<ChannelInfo> channels;
    QXmlStreamReader reader(xml);

    ChannelInfo currentChannel;
    bool inDeviceList = false;
    bool inItem = false;

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement()) {
            QString name = reader.name().toString();

            if (name == "DeviceList") {
                inDeviceList = true;
            } else if (name == "Item" && inDeviceList) {
                inItem = true;
                currentChannel = ChannelInfo();
                currentChannel.deviceId = deviceId;
            } else if (inItem) {
                QString text = reader.readElementText();

                if (name == "DeviceID") {
                    currentChannel.channelId = text;
                } else if (name == "Name") {
                    currentChannel.name = text;
                } else if (name == "Manufacturer") {
                    currentChannel.manufacturer = text;
                } else if (name == "Model") {
                    currentChannel.model = text;
                } else if (name == "Status") {
                    currentChannel.status = (text == "ON" || text == "Online")
                                           ? ChannelStatus::Online
                                           : ChannelStatus::Offline;
                } else if (name == "ParentID") {
                    currentChannel.parentId = text;
                } else if (name == "Longitude") {
                    currentChannel.longitude = text;
                } else if (name == "Latitude") {
                    currentChannel.latitude = text;
                } else if (name == "PTZType") {
                    currentChannel.ptzSupport = (text.toInt() > 0);
                }
            }
        } else if (reader.isEndElement()) {
            QString name = reader.name().toString();

            if (name == "Item" && inItem) {
                channels.append(currentChannel);
                inItem = false;
            } else if (name == "DeviceList") {
                inDeviceList = false;
            }
        }
    }

    qDebug() << "Parsed" << channels.size() << "channels from device" << deviceId;

    emit catalogReceived(deviceId, channels);
}

void GB28181Server::parseRecordInfo(const QString &deviceId, const QString &xml)
{
    QList<RecordFileInfo> records;
    QXmlStreamReader reader(xml);

    RecordFileInfo currentRecord;
    bool inRecordList = false;
    bool inItem = false;

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement()) {
            QString name = reader.name().toString();

            if (name == "RecordList") {
                inRecordList = true;
            } else if (name == "Item" && inRecordList) {
                inItem = true;
                currentRecord = RecordFileInfo();
                currentRecord.deviceId = deviceId;
            } else if (inItem) {
                QString text = reader.readElementText();

                if (name == "DeviceID") {
                    // 通道ID
                } else if (name == "Name") {
                    currentRecord.name = text;
                } else if (name == "FilePath") {
                    currentRecord.filePath = text;
                } else if (name == "Address") {
                    currentRecord.address = text;
                } else if (name == "StartTime") {
                    currentRecord.startTime = QDateTime::fromString(text, Qt::ISODate);
                } else if (name == "EndTime") {
                    currentRecord.endTime = QDateTime::fromString(text, Qt::ISODate);
                } else if (name == "Secrecy") {
                    currentRecord.secrecy = text.toInt();
                } else if (name == "Type") {
                    currentRecord.type = text;
                }
            }
        } else if (reader.isEndElement()) {
            QString name = reader.name().toString();

            if (name == "Item" && inItem) {
                records.append(currentRecord);
                inItem = false;
            } else if (name == "RecordList") {
                inRecordList = false;
            }
        }
    }

    emit recordInfoReceived(deviceId, records);
}

bool GB28181Server::sendCatalogQuery(const QString &deviceId)
{
    if (!m_exosipContext) {
        return false;
    }

    QString xml = generateCatalogQuery(deviceId);

    // 构造 MESSAGE 请求
    osip_message_t *message = nullptr;
    char from[256], to[256];

    snprintf(from, sizeof(from), "sip:%s@%s:%d",
             m_config.serverId.toUtf8().constData(),
             m_config.serverIp.toUtf8().constData(),
             m_config.serverPort);

    snprintf(to, sizeof(to), "sip:%s@%s",
             deviceId.toUtf8().constData(),
             m_config.domain.toUtf8().constData());

    eXosip_lock(m_exosipContext);
    eXosip_message_build_request(m_exosipContext, &message, "MESSAGE", to, from, nullptr);

    if (message) {
        osip_message_set_content_type(message, "Application/MANSCDP+xml");
        osip_message_set_body(message, xml.toUtf8().constData(), xml.toUtf8().length());

        eXosip_message_send_request(m_exosipContext, message);
    }

    eXosip_unlock(m_exosipContext);

    qDebug() << "Sent Catalog query to" << deviceId;

    return true;
}

bool GB28181Server::sendInvite(const QString &deviceId, const QString &channelId,
                                const QString &localIp, int localPort)
{
    if (!m_exosipContext) {
        return false;
    }

    // 生成 SSRC
    QString ssrc = QString("0%1").arg(QDateTime::currentMSecsSinceEpoch() % 1000000000);

    QString sdp = generateSDP(localIp, localPort, ssrc);

    char from[256], to[256], subject[256];

    snprintf(from, sizeof(from), "sip:%s@%s:%d",
             m_config.serverId.toUtf8().constData(),
             m_config.serverIp.toUtf8().constData(),
             m_config.serverPort);

    snprintf(to, sizeof(to), "sip:%s@%s",
             channelId.toUtf8().constData(),
             m_config.domain.toUtf8().constData());

    snprintf(subject, sizeof(subject), "%s:%s,%s:0",
             channelId.toUtf8().constData(),
             ssrc.toUtf8().constData(),
             m_config.serverId.toUtf8().constData());

    osip_message_t *invite = nullptr;

    eXosip_lock(m_exosipContext);

    int callId = eXosip_call_build_initial_invite(m_exosipContext, &invite, to, from, nullptr, subject);

    if (invite && callId > 0) {
        osip_message_set_content_type(invite, "application/sdp");
        osip_message_set_body(invite, sdp.toUtf8().constData(), sdp.toUtf8().length());

        eXosip_call_send_initial_invite(m_exosipContext, invite);

        qDebug() << "Sent INVITE to" << channelId << "call_id:" << callId;
    }

    eXosip_unlock(m_exosipContext);

    return callId > 0;
}

bool GB28181Server::sendBye(int callId, int dialogId)
{
    if (!m_exosipContext || callId < 0 || dialogId < 0) {
        return false;
    }

    eXosip_lock(m_exosipContext);
    eXosip_call_terminate(m_exosipContext, callId, dialogId);
    eXosip_unlock(m_exosipContext);

    m_sessions.remove(callId);

    qDebug() << "Sent BYE for call_id:" << callId;

    return true;
}

bool GB28181Server::sendPTZControl(const QString &deviceId, const QString &channelId,
                                    PTZCommand command, int speed)
{
    if (!m_exosipContext) {
        return false;
    }

    QString xml = generatePTZControl(command, speed);

    osip_message_t *message = nullptr;
    char from[256], to[256];

    snprintf(from, sizeof(from), "sip:%s@%s:%d",
             m_config.serverId.toUtf8().constData(),
             m_config.serverIp.toUtf8().constData(),
             m_config.serverPort);

    snprintf(to, sizeof(to), "sip:%s@%s",
             channelId.toUtf8().constData(),
             m_config.domain.toUtf8().constData());

    eXosip_lock(m_exosipContext);
    eXosip_message_build_request(m_exosipContext, &message, "MESSAGE", to, from, nullptr);

    if (message) {
        osip_message_set_content_type(message, "Application/MANSCDP+xml");
        osip_message_set_body(message, xml.toUtf8().constData(), xml.toUtf8().length());

        eXosip_message_send_request(m_exosipContext, message);
    }

    eXosip_unlock(m_exosipContext);

    return true;
}

QString GB28181Server::generateCatalogQuery(const QString &deviceId)
{
    QString sn = generateSN();

    QString xml = QString(
        "<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n"
        "<Query>\r\n"
        "<CmdType>Catalog</CmdType>\r\n"
        "<SN>%1</SN>\r\n"
        "<DeviceID>%2</DeviceID>\r\n"
        "</Query>\r\n"
    ).arg(sn).arg(deviceId);

    return xml;
}

QString GB28181Server::generatePTZControl(PTZCommand command, int speed)
{
    // PTZ 控制字节
    unsigned char byte1 = 0xA5;
    unsigned char byte2 = 0x0F;
    unsigned char byte3 = 0x01;
    unsigned char byte4 = 0x00;
    unsigned char byte5 = (speed & 0xFF);
    unsigned char byte6 = (speed & 0xFF);
    unsigned char byte7 = 0x00;
    unsigned char byte8 = (byte1 + byte2 + byte3 + byte4 + byte5 + byte6 + byte7) % 256;

    // 根据命令设置控制字节
    switch (command) {
    case PTZCommand::Up:
        byte4 = 0x08;
        break;
    case PTZCommand::Down:
        byte4 = 0x04;
        break;
    case PTZCommand::Left:
        byte4 = 0x02;
        break;
    case PTZCommand::Right:
        byte4 = 0x01;
        break;
    case PTZCommand::ZoomIn:
        byte4 = 0x10;
        break;
    case PTZCommand::ZoomOut:
        byte4 = 0x20;
        break;
    default:
        byte4 = 0x00;
        break;
    }

    byte8 = (byte1 + byte2 + byte3 + byte4 + byte5 + byte6 + byte7) % 256;

    QString ptzCmd = QString("%1%2%3%4%5%6%7%8")
        .arg(byte1, 2, 16, QChar('0'))
        .arg(byte2, 2, 16, QChar('0'))
        .arg(byte3, 2, 16, QChar('0'))
        .arg(byte4, 2, 16, QChar('0'))
        .arg(byte5, 2, 16, QChar('0'))
        .arg(byte6, 2, 16, QChar('0'))
        .arg(byte7, 2, 16, QChar('0'))
        .arg(byte8, 2, 16, QChar('0'))
        .toUpper();

    QString sn = generateSN();

    QString xml = QString(
        "<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n"
        "<Control>\r\n"
        "<CmdType>DeviceControl</CmdType>\r\n"
        "<SN>%1</SN>\r\n"
        "<DeviceID>%2</DeviceID>\r\n"
        "<PTZCmd>%3</PTZCmd>\r\n"
        "</Control>\r\n"
    ).arg(sn).arg("").arg(ptzCmd);

    return xml;
}

QString GB28181Server::generateSDP(const QString &localIp, int localPort, const QString &ssrc)
{
    QString sdp = QString(
        "v=0\r\n"
        "o=%1 0 0 IN IP4 %2\r\n"
        "s=Play\r\n"
        "c=IN IP4 %2\r\n"
        "t=0 0\r\n"
        "m=video %3 RTP/AVP 96 98 97\r\n"
        "a=rtpmap:96 PS/90000\r\n"
        "a=rtpmap:98 H264/90000\r\n"
        "a=rtpmap:97 MPEG4/90000\r\n"
        "a=recvonly\r\n"
        "a=ssrc:%4\r\n"
        "y=%4\r\n"
    ).arg(m_config.serverId)
     .arg(localIp)
     .arg(localPort)
     .arg(ssrc);

    return sdp;
}

QString GB28181Server::generateSN()
{
    QMutexLocker locker(&m_mutex);
    return QString::number(m_snCounter++);
}

QString GB28181Server::extractDeviceId(const QString &from)
{
    // 从 SIP URI 中提取设备ID
    return from;
}

QString GB28181Server::extractXmlBody(osip_message_t *message)
{
    if (!message || !message->bodies.node) {
        return QString();
    }

    osip_body_t *body = (osip_body_t *)message->bodies.node->element;
    if (!body || !body->body) {
        return QString();
    }

    return QString::fromUtf8(body->body);
}

} // namespace GB28181
