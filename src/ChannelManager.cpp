#include "ChannelManager.h"
#include <QDebug>

namespace GB28181 {

ChannelManager::ChannelManager(QObject *parent)
    : QObject(parent)
{
}

ChannelManager::~ChannelManager()
{
}

void ChannelManager::addChannel(const ChannelInfo &channel)
{
    if (m_channels.contains(channel.channelId)) {
        // 更新已存在的通道
        m_channels[channel.channelId] = channel;
        return;
    }

    m_channels[channel.channelId] = channel;

    // 添加到设备通道列表
    if (!m_deviceChannels.contains(channel.deviceId)) {
        m_deviceChannels[channel.deviceId] = QList<QString>();
    }
    m_deviceChannels[channel.deviceId].append(channel.channelId);

    emit channelAdded(channel);

    qDebug() << "Channel added:" << channel.channelId << channel.name;
}

void ChannelManager::removeChannel(const QString &channelId)
{
    if (!m_channels.contains(channelId)) {
        return;
    }

    ChannelInfo channel = m_channels[channelId];
    m_channels.remove(channelId);

    // 从设备通道列表中移除
    if (m_deviceChannels.contains(channel.deviceId)) {
        m_deviceChannels[channel.deviceId].removeAll(channelId);
    }

    emit channelRemoved(channelId);

    qDebug() << "Channel removed:" << channelId;
}

void ChannelManager::clearChannels(const QString &deviceId)
{
    if (!m_deviceChannels.contains(deviceId)) {
        return;
    }

    QList<QString> channelIds = m_deviceChannels[deviceId];

    for (const QString &channelId : channelIds) {
        m_channels.remove(channelId);
        emit channelRemoved(channelId);
    }

    m_deviceChannels.remove(deviceId);

    qDebug() << "Cleared" << channelIds.size() << "channels for device" << deviceId;
}

void ChannelManager::updateChannelStatus(const QString &channelId, ChannelStatus status)
{
    if (!m_channels.contains(channelId)) {
        return;
    }

    ChannelInfo &channel = m_channels[channelId];
    if (channel.status != status) {
        channel.status = status;
        emit channelStatusChanged(channelId, status);

        qDebug() << "Channel status changed:" << channelId << (int)status;
    }
}

ChannelInfo ChannelManager::getChannel(const QString &channelId) const
{
    return m_channels.value(channelId);
}

QList<ChannelInfo> ChannelManager::getDeviceChannels(const QString &deviceId) const
{
    QList<ChannelInfo> channels;

    if (m_deviceChannels.contains(deviceId)) {
        const QList<QString> &channelIds = m_deviceChannels[deviceId];
        for (const QString &channelId : channelIds) {
            if (m_channels.contains(channelId)) {
                channels.append(m_channels[channelId]);
            }
        }
    }

    return channels;
}

QList<ChannelInfo> ChannelManager::getAllChannels() const
{
    return m_channels.values();
}

bool ChannelManager::hasChannel(const QString &channelId) const
{
    return m_channels.contains(channelId);
}

} // namespace GB28181
