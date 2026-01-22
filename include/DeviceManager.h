#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include "GB28181Types.h"

namespace GB28181 {

class DeviceManager : public QObject
{
    Q_OBJECT

public:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager();

    // 添加/删除设备
    void addDevice(const DeviceInfo &device);
    void removeDevice(const QString &deviceId);

    // 更新设备信息
    void updateDeviceStatus(const QString &deviceId, DeviceStatus status);
    void updateDeviceKeepalive(const QString &deviceId);

    // 获取设备信息
    DeviceInfo getDevice(const QString &deviceId) const;
    QList<DeviceInfo> getAllDevices() const;
    bool hasDevice(const QString &deviceId) const;

    // 设置心跳超时参数
    void setKeepaliveTimeout(int seconds, int maxTimeout);

signals:
    // 设备状态变化
    void deviceAdded(const DeviceInfo &device);
    void deviceRemoved(const QString &deviceId);
    void deviceStatusChanged(const QString &deviceId, DeviceStatus status);

    // 设备超时
    void deviceTimeout(const QString &deviceId);

private slots:
    void checkKeepaliveTimeout();

private:
    QMap<QString, DeviceInfo> m_devices;
    QTimer *m_timeoutTimer;
    int m_keepaliveTimeout;      // 心跳超时时间(秒)
    int m_maxTimeoutCount;       // 最大超时次数
};

} // namespace GB28181

#endif // DEVICE_MANAGER_H
