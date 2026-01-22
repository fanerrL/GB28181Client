#ifndef RTP_RECEIVER_H
#define RTP_RECEIVER_H

#include <QObject>
#include <QUdpSocket>
#include <QMap>
#include <QByteArray>

namespace GB28181 {

// RTP 头结构
#pragma pack(push, 1)
struct RtpHeader {
    uint8_t cc:4;        // CSRC count
    uint8_t x:1;         // extension
    uint8_t p:1;         // padding
    uint8_t v:2;         // version
    uint8_t pt:7;        // payload type
    uint8_t m:1;         // marker
    uint16_t seq;        // sequence number
    uint32_t timestamp;  // timestamp
    uint32_t ssrc;       // SSRC
};
#pragma pack(pop)

class RtpReceiver : public QObject
{
    Q_OBJECT

public:
    explicit RtpReceiver(QObject *parent = nullptr);
    ~RtpReceiver();

    // 启动接收
    bool start(const QString &ip, int port);
    void stop();

    bool isRunning() const { return m_running; }
    int port() const { return m_port; }

signals:
    // RTP 包接收
    void rtpPacketReceived(const QByteArray &payload, uint32_t timestamp, uint16_t seq);

    // PS 流数据
    void psStreamData(const QByteArray &data);

    // 错误
    void errorOccurred(const QString &error);

private slots:
    void onReadyRead();

private:
    void processRtpPacket(const QByteArray &packet);
    void processPsStream(const QByteArray &payload);

private:
    QUdpSocket *m_socket;
    QString m_ip;
    int m_port;
    bool m_running;

    // RTP 包重组
    QMap<uint16_t, QByteArray> m_packetBuffer;
    uint16_t m_lastSeq;
    QByteArray m_psBuffer;
};

} // namespace GB28181

#endif // RTP_RECEIVER_H
