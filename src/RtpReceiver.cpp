#include "RtpReceiver.h"
#include <QDebug>
#include <arpa/inet.h>

namespace GB28181 {

RtpReceiver::RtpReceiver(QObject *parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_port(0)
    , m_running(false)
    , m_lastSeq(0)
{
}

RtpReceiver::~RtpReceiver()
{
    stop();
}

bool RtpReceiver::start(const QString &ip, int port)
{
    if (m_running) {
        return false;
    }

    m_ip = ip;
    m_port = port;

    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead, this, &RtpReceiver::onReadyRead);

    if (!m_socket->bind(QHostAddress(ip), port)) {
        emit errorOccurred(QString("Failed to bind to %1:%2").arg(ip).arg(port));
        delete m_socket;
        m_socket = nullptr;
        return false;
    }

    m_running = true;
    qDebug() << "RTP Receiver started on" << ip << ":" << port;

    return true;
}

void RtpReceiver::stop()
{
    if (!m_running) {
        return;
    }

    m_running = false;

    if (m_socket) {
        m_socket->close();
        delete m_socket;
        m_socket = nullptr;
    }

    m_packetBuffer.clear();
    m_psBuffer.clear();

    qDebug() << "RTP Receiver stopped";
}

void RtpReceiver::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        m_socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        processRtpPacket(datagram);
    }
}

void RtpReceiver::processRtpPacket(const QByteArray &packet)
{
    if (packet.size() < sizeof(RtpHeader)) {
        return;
    }

    const RtpHeader *header = reinterpret_cast<const RtpHeader*>(packet.constData());

    // 转换字节序
    uint16_t seq = ntohs(header->seq);
    uint32_t timestamp = ntohl(header->timestamp);
    uint32_t ssrc = ntohl(header->ssrc);

    // 计算 RTP 头长度
    int headerLen = sizeof(RtpHeader);
    if (header->cc > 0) {
        headerLen += header->cc * 4;
    }
    if (header->x) {
        // 扩展头
        if (packet.size() < headerLen + 4) {
            return;
        }
        uint16_t extLen = ntohs(*reinterpret_cast<const uint16_t*>(packet.constData() + headerLen + 2));
        headerLen += 4 + extLen * 4;
    }

    if (packet.size() <= headerLen) {
        return;
    }

    // 提取 payload
    QByteArray payload = packet.mid(headerLen);

    emit rtpPacketReceived(payload, timestamp, seq);

    // 处理 PS 流
    processPsStream(payload);
}

void RtpReceiver::processPsStream(const QByteArray &payload)
{
    // 将 RTP payload 添加到 PS 缓冲区
    m_psBuffer.append(payload);

    // 查找 PS 包起始码 0x000001BA
    while (m_psBuffer.size() >= 4) {
        int startPos = -1;

        for (int i = 0; i <= m_psBuffer.size() - 4; ++i) {
            if ((unsigned char)m_psBuffer[i] == 0x00 &&
                (unsigned char)m_psBuffer[i+1] == 0x00 &&
                (unsigned char)m_psBuffer[i+2] == 0x01 &&
                (unsigned char)m_psBuffer[i+3] == 0xBA) {
                startPos = i;
                break;
            }
        }

        if (startPos < 0) {
            // 没有找到起始码，保留最后3个字节
            if (m_psBuffer.size() > 3) {
                m_psBuffer = m_psBuffer.right(3);
            }
            break;
        }

        if (startPos > 0) {
            // 丢弃起始码之前的数据
            m_psBuffer = m_psBuffer.mid(startPos);
        }

        // 查找下一个 PS 包起始码
        int nextPos = -1;
        for (int i = 4; i <= m_psBuffer.size() - 4; ++i) {
            if ((unsigned char)m_psBuffer[i] == 0x00 &&
                (unsigned char)m_psBuffer[i+1] == 0x00 &&
                (unsigned char)m_psBuffer[i+2] == 0x01 &&
                (unsigned char)m_psBuffer[i+3] == 0xBA) {
                nextPos = i;
                break;
            }
        }

        if (nextPos > 0) {
            // 提取完整的 PS 包
            QByteArray psPacket = m_psBuffer.left(nextPos);
            m_psBuffer = m_psBuffer.mid(nextPos);

            emit psStreamData(psPacket);
        } else {
            // 没有找到下一个起始码，等待更多数据
            break;
        }
    }
}

} // namespace GB28181
