#ifndef CHANNEL_MANAGER_H
#define CHANNEL_MANAGER_H

#include <QObject>
#include <QMap>
#include <QList>
#include "GB28181Types.h"

namespace GB28181 {

class ChannelManager : public QObject
{
    Q_OBJECT

public:
    explicit ChannelManager(QObject *parent = nullptr);
    ~ChannelManager();

    // 添加/删除通道
    void addChannel(const ChannelInfo &channel);
    void removeChannel(const QString &channelId);
    void clearChannels(const QString &deviceId);

    // 更新通道信息
    void updateChannelStatus(const QString &channelId, ChannelStatus status);

    // 获取通道信息
    ChannelInfo getChannel(const QString &channelId) const;
    QList<ChannelInfo> getDeviceChannels(const QString &deviceId) const;
    QList<ChannelInfo> getAllChannels() const;
    bool hasChannel(const QString &channelId) const;

signals:
    // 通道变化
    void channelAdded(const ChannelInfo &channel);
    void channelRemoved(const QString &channelId);
    void channelStatusChanged(const QString &channelId, ChannelStatus status);

private:
    QMap<QString, ChannelInfo> m_channels;  // channelId -> ChannelInfo
    QMap<QString, QList<QString>> m_deviceChannels;  // deviceId -> [channelIds]
};

} // namespace GB28181

#endif // CHANNEL_MANAGER_H
