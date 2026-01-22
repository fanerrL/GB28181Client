#include "DeviceManager.h"
#include <QDebug>

namespace GB28181 {

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
    , m_keepaliveTimeout(60)
    , m_maxTimeoutCount(3)
{
    m_timeoutTimer = new QTimer(this);
    connect(m_timeoutTimer, &QTimer::timeout, this, &DeviceManager::checkKeepaliveTimeout);
    m_timeoutTimer->start(10000); // 每10秒检查一次
}

DeviceManager::~DeviceManager()
{
}

void DeviceManager::addDevice(const DeviceInfo &device)
{
    if (m_devices.contains(device.deviceId)) {
        qDebug() << "Device already exists:" << device.deviceId;
        return;
    }

    m_devices[device.deviceId] = device;
    emit deviceAdded(device);

    qDebug() << "Device added:" << device.deviceId << device.name;
}

void DeviceManager::removeDevice(const QString &deviceId)
{
    if (!m_devices.contains(deviceId)) {
        return;
    }

    m_devices.remove(deviceId);
    emit deviceRemoved(deviceId);

    qDebug() << "Device removed:" << deviceId;
}

void DeviceManager::updateDeviceStatus(const QString &deviceId, DeviceStatus status)
{
    if (!m_devices.contains(deviceId)) {
        return;
    }

    DeviceInfo &device = m_devices[deviceId];
    if (device.status != status) {
        device.status = status;
        emit deviceStatusChanged(deviceId, status);

        qDebug() << "Device status changed:" << deviceId << (int)status;
    }
}

void DeviceManager::updateDeviceKeepalive(const QString &deviceId)
{
    if (!m_devices.contains(deviceId)) {
        return;
    }

    DeviceInfo &device = m_devices[deviceId];
    device.lastKeepalive = QDateTime::currentDateTime();
    device.keepaliveTimeout = 0;

    // 更新状态为在线
    if (device.status != DeviceStatus::Online) {
        updateDeviceStatus(deviceId, DeviceStatus::Online);
    }
}

DeviceInfo DeviceManager::getDevice(const QString &deviceId) const
{
    return m_devices.value(deviceId);
}

QList<DeviceInfo> DeviceManager::getAllDevices() const
{
    return m_devices.values();
}

bool DeviceManager::hasDevice(const QString &deviceId) const
{
    return m_devices.contains(deviceId);
}

void DeviceManager::setKeepaliveTimeout(int seconds, int maxTimeout)
{
    m_keepaliveTimeout = seconds;
    m_maxTimeoutCount = maxTimeout;
}

void DeviceManager::checkKeepaliveTimeout()
{
    QDateTime now = QDateTime::currentDateTime();

    for (auto it = m_devices.begin(); it != m_devices.end(); ++it) {
        DeviceInfo &device = it.value();

        if (device.status != DeviceStatus::Online) {
            continue;
        }

        qint64 elapsed = device.lastKeepalive.secsTo(now);

        if (elapsed > m_keepaliveTimeout) {
            device.keepaliveTimeout++;

            qDebug() << "Device keepalive timeout:" << device.deviceId
                     << "count:" << device.keepaliveTimeout;

            if (device.keepaliveTimeout >= m_maxTimeoutCount) {
                // 设备超时
                updateDeviceStatus(device.deviceId, DeviceStatus::Offline);
                emit deviceTimeout(device.deviceId);
            }
        }
    }
}

} // namespace GB28181
