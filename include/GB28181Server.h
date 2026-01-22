#ifndef GB28181_SERVER_H
#define GB28181_SERVER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include "GB28181Types.h"

extern "C" {
#include <eXosip2/eXosip.h>
}

namespace GB28181 {

class GB28181Server : public QObject
{
    Q_OBJECT

public:
    explicit GB28181Server(QObject *parent = nullptr);
    ~GB28181Server();

    // 启动/停止服务器
    bool start(const GB28181Config &config);
    void stop();
    bool isRunning() const { return m_running; }

    // 获取配置
    const GB28181Config& config() const { return m_config; }

    // 发送 Catalog 查询
    bool sendCatalogQuery(const QString &deviceId);

    // 发送视频点播 INVITE
    bool sendInvite(const QString &deviceId, const QString &channelId,
                    const QString &localIp, int localPort);

    // 发送 BYE 停止视频
    bool sendBye(int callId, int dialogId);

    // 发送 PTZ 控制
    bool sendPTZControl(const QString &deviceId, const QString &channelId,
                        PTZCommand command, int speed);

    // 发送录像查询
    bool sendRecordInfoQuery(const QString &deviceId, const QString &channelId,
                             const QDateTime &startTime, const QDateTime &endTime);

signals:
    // 设备注册
    void deviceRegistered(const DeviceInfo &device);
    void deviceUnregistered(const QString &deviceId);

    // 心跳
    void keepaliveReceived(const QString &deviceId);

    // 通道目录
    void catalogReceived(const QString &deviceId, const QList<ChannelInfo> &channels);

    // 媒体流信息
    void mediaStreamReady(const SipSession &session);

    // 录像信息
    void recordInfoReceived(const QString &deviceId, const QList<RecordFileInfo> &records);

    // 错误
    void errorOccurred(const QString &error);

private slots:
    void processEvents();

private:
    // 初始化 eXosip
    bool initEXosip();
    void cleanupEXosip();

    // 处理 SIP 消息
    void handleRegister(eXosip_event_t *event);
    void handleMessage(eXosip_event_t *event);
    void handleInviteAnswered(eXosip_event_t *event);
    void handleCallClosed(eXosip_event_t *event);

    // 解析 XML 消息
    void parseKeepalive(const QString &deviceId, const QString &xml);
    void parseCatalog(const QString &deviceId, const QString &xml);
    void parseRecordInfo(const QString &deviceId, const QString &xml);

    // 生成 XML 消息
    QString generateCatalogQuery(const QString &deviceId);
    QString generatePTZControl(PTZCommand command, int speed);
    QString generateRecordInfoQuery(const QString &channelId,
                                     const QDateTime &startTime,
                                     const QDateTime &endTime);

    // 生成 SDP
    QString generateSDP(const QString &localIp, int localPort, const QString &ssrc);

    // 工具函数
    QString generateSN();
    QString extractDeviceId(const QString &from);
    QString extractXmlBody(osip_message_t *message);

private:
    struct eXosip_t *m_exosipContext;
    GB28181Config m_config;
    bool m_running;
    QThread *m_eventThread;
    QTimer *m_eventTimer;
    QMutex m_mutex;
    int m_snCounter;

    // 会话管理
    QMap<int, SipSession> m_sessions;  // callId -> session
};

} // namespace GB28181

#endif // GB28181_SERVER_H
