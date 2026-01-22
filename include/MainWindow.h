#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include "GB28181Server.h"
#include "DeviceManager.h"
#include "ChannelManager.h"
#include "RtpReceiver.h"
#include "PsStreamParser.h"
#include "VideoDecoder.h"

namespace GB28181 {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 服务器控制
    void onStartServer();
    void onStopServer();

    // 设备管理
    void onDeviceRegistered(const DeviceInfo &device);
    void onDeviceRemoved(const QString &deviceId);
    void onKeepaliveReceived(const QString &deviceId);

    // 通道管理
    void onCatalogReceived(const QString &deviceId, const QList<ChannelInfo> &channels);
    void onQueryCatalog();

    // 视频播放
    void onStartPlay();
    void onStopPlay();
    void onMediaStreamReady(const SipSession &session);
    void onVideoFrameDecoded(const QImage &image);

    // PTZ 控制
    void onPTZUp();
    void onPTZDown();
    void onPTZLeft();
    void onPTZRight();
    void onPTZZoomIn();
    void onPTZZoomOut();
    void onPTZStop();

    // 树形视图
    void onDeviceTreeItemClicked(QTreeWidgetItem *item, int column);

    // 日志
    void appendLog(const QString &message);

private:
    void setupUI();
    void setupConnections();
    void updateDeviceTree();
    void updateChannelTree(const QString &deviceId);

private:
    // UI 组件
    QWidget *m_centralWidget;
    QTreeWidget *m_deviceTree;
    QLabel *m_videoLabel;
    QPushButton *m_startServerBtn;
    QPushButton *m_stopServerBtn;
    QPushButton *m_queryCatalogBtn;
    QPushButton *m_startPlayBtn;
    QPushButton *m_stopPlayBtn;
    QLineEdit *m_serverIdEdit;
    QLineEdit *m_serverIpEdit;
    QSpinBox *m_serverPortSpin;
    QTextEdit *m_logText;

    // PTZ 控制按钮
    QPushButton *m_ptzUpBtn;
    QPushButton *m_ptzDownBtn;
    QPushButton *m_ptzLeftBtn;
    QPushButton *m_ptzRightBtn;
    QPushButton *m_ptzZoomInBtn;
    QPushButton *m_ptzZoomOutBtn;

    // 核心组件
    GB28181Server *m_server;
    DeviceManager *m_deviceManager;
    ChannelManager *m_channelManager;
    RtpReceiver *m_rtpReceiver;
    PsStreamParser *m_psParser;
    VideoDecoder *m_videoDecoder;

    // 当前选中
    QString m_selectedDeviceId;
    QString m_selectedChannelId;

    // 当前会话
    SipSession m_currentSession;
};

} // namespace GB28181

#endif // MAIN_WINDOW_H
