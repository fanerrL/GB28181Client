#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QMessageBox>
#include <QDateTime>

namespace GB28181 {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_server(nullptr)
    , m_deviceManager(nullptr)
    , m_channelManager(nullptr)
    , m_rtpReceiver(nullptr)
    , m_psParser(nullptr)
    , m_videoDecoder(nullptr)
{
    setupUI();

    // 创建核心组件
    m_server = new GB28181Server(this);
    m_deviceManager = new DeviceManager(this);
    m_channelManager = new ChannelManager(this);
    m_rtpReceiver = new RtpReceiver(this);
    m_psParser = new PsStreamParser(this);
    m_videoDecoder = new VideoDecoder(this);

    setupConnections();

    setWindowTitle("GB28181 级联上级客户端");
    resize(1280, 720);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_centralWidget);

    // 服务器配置区域
    QGroupBox *serverGroup = new QGroupBox("服务器配置", this);
    QHBoxLayout *serverLayout = new QHBoxLayout(serverGroup);

    serverLayout->addWidget(new QLabel("平台ID:"));
    m_serverIdEdit = new QLineEdit("34020000002000000001");
    serverLayout->addWidget(m_serverIdEdit);

    serverLayout->addWidget(new QLabel("IP:"));
    m_serverIpEdit = new QLineEdit("0.0.0.0");
    serverLayout->addWidget(m_serverIpEdit);

    serverLayout->addWidget(new QLabel("端口:"));
    m_serverPortSpin = new QSpinBox();
    m_serverPortSpin->setRange(1024, 65535);
    m_serverPortSpin->setValue(5060);
    serverLayout->addWidget(m_serverPortSpin);

    m_startServerBtn = new QPushButton("启动服务");
    m_stopServerBtn = new QPushButton("停止服务");
    m_stopServerBtn->setEnabled(false);
    serverLayout->addWidget(m_startServerBtn);
    serverLayout->addWidget(m_stopServerBtn);

    serverLayout->addStretch();

    mainLayout->addWidget(serverGroup);

    // 主内容区域
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    // 左侧：设备树
    QWidget *leftWidget = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);

    m_deviceTree = new QTreeWidget();
    m_deviceTree->setHeaderLabels({"设备/通道", "状态", "ID"});
    m_deviceTree->setColumnWidth(0, 200);
    leftLayout->addWidget(m_deviceTree);

    m_queryCatalogBtn = new QPushButton("查询通道");
    m_queryCatalogBtn->setEnabled(false);
    leftLayout->addWidget(m_queryCatalogBtn);

    splitter->addWidget(leftWidget);

    // 中间：视频显示
    QWidget *centerWidget = new QWidget();
    QVBoxLayout *centerLayout = new QVBoxLayout(centerWidget);

    m_videoLabel = new QLabel();
    m_videoLabel->setMinimumSize(640, 480);
    m_videoLabel->setStyleSheet("QLabel { background-color: black; }");
    m_videoLabel->setScaledContents(true);
    centerLayout->addWidget(m_videoLabel);

    QHBoxLayout *playLayout = new QHBoxLayout();
    m_startPlayBtn = new QPushButton("开始播放");
    m_startPlayBtn->setEnabled(false);
    m_stopPlayBtn = new QPushButton("停止播放");
    m_stopPlayBtn->setEnabled(false);
    playLayout->addWidget(m_startPlayBtn);
    playLayout->addWidget(m_stopPlayBtn);
    playLayout->addStretch();
    centerLayout->addLayout(playLayout);

    splitter->addWidget(centerWidget);

    // 右侧：PTZ 控制和日志
    QWidget *rightWidget = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);

    // PTZ 控制
    QGroupBox *ptzGroup = new QGroupBox("云台控制");
    QGridLayout *ptzLayout = new QGridLayout(ptzGroup);

    m_ptzUpBtn = new QPushButton("↑");
    m_ptzDownBtn = new QPushButton("↓");
    m_ptzLeftBtn = new QPushButton("←");
    m_ptzRightBtn = new QPushButton("→");
    m_ptzZoomInBtn = new QPushButton("放大");
    m_ptzZoomOutBtn = new QPushButton("缩小");

    ptzLayout->addWidget(m_ptzUpBtn, 0, 1);
    ptzLayout->addWidget(m_ptzLeftBtn, 1, 0);
    ptzLayout->addWidget(new QLabel("方向"), 1, 1, Qt::AlignCenter);
    ptzLayout->addWidget(m_ptzRightBtn, 1, 2);
    ptzLayout->addWidget(m_ptzDownBtn, 2, 1);
    ptzLayout->addWidget(m_ptzZoomInBtn, 3, 0);
    ptzLayout->addWidget(m_ptzZoomOutBtn, 3, 2);

    rightLayout->addWidget(ptzGroup);

    // 日志
    QGroupBox *logGroup = new QGroupBox("日志");
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    m_logText = new QTextEdit();
    m_logText->setReadOnly(true);
    logLayout->addWidget(m_logText);
    rightLayout->addWidget(logGroup);

    splitter->addWidget(rightWidget);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setStretchFactor(2, 1);

    mainLayout->addWidget(splitter);
}

void MainWindow::setupConnections()
{
    // 服务器控制
    connect(m_startServerBtn, &QPushButton::clicked, this, &MainWindow::onStartServer);
    connect(m_stopServerBtn, &QPushButton::clicked, this, &MainWindow::onStopServer);

    // 服务器信号
    connect(m_server, &GB28181Server::deviceRegistered, this, &MainWindow::onDeviceRegistered);
    connect(m_server, &GB28181Server::deviceUnregistered, this, &MainWindow::onDeviceRemoved);
    connect(m_server, &GB28181Server::keepaliveReceived, this, &MainWindow::onKeepaliveReceived);
    connect(m_server, &GB28181Server::catalogReceived, this, &MainWindow::onCatalogReceived);
    connect(m_server, &GB28181Server::mediaStreamReady, this, &MainWindow::onMediaStreamReady);

    // 设备管理
    connect(m_deviceManager, &DeviceManager::deviceAdded,
            [this](const DeviceInfo &) { updateDeviceTree(); });
    connect(m_deviceManager, &DeviceManager::deviceRemoved,
            [this](const QString &) { updateDeviceTree(); });

    // 通道管理
    connect(m_queryCatalogBtn, &QPushButton::clicked, this, &MainWindow::onQueryCatalog);

    // 视频播放
    connect(m_startPlayBtn, &QPushButton::clicked, this, &MainWindow::onStartPlay);
    connect(m_stopPlayBtn, &QPushButton::clicked, this, &MainWindow::onStopPlay);

    // RTP 接收
    connect(m_rtpReceiver, &RtpReceiver::psStreamData, m_psParser, &PsStreamParser::parse);

    // PS 解析
    connect(m_psParser, &PsStreamParser::videoFrame,
            [this](const QByteArray &frame, VideoCodec codec, bool) {
                m_videoDecoder->decode(frame);
            });

    // 视频解码
    connect(m_videoDecoder, &VideoDecoder::frameDecoded, this, &MainWindow::onVideoFrameDecoded);

    // PTZ 控制
    connect(m_ptzUpBtn, &QPushButton::clicked, this, &MainWindow::onPTZUp);
    connect(m_ptzDownBtn, &QPushButton::clicked, this, &MainWindow::onPTZDown);
    connect(m_ptzLeftBtn, &QPushButton::clicked, this, &MainWindow::onPTZLeft);
    connect(m_ptzRightBtn, &QPushButton::clicked, this, &MainWindow::onPTZRight);
    connect(m_ptzZoomInBtn, &QPushButton::clicked, this, &MainWindow::onPTZZoomIn);
    connect(m_ptzZoomOutBtn, &QPushButton::clicked, this, &MainWindow::onPTZZoomOut);

    // 设备树点击
    connect(m_deviceTree, &QTreeWidget::itemClicked, this, &MainWindow::onDeviceTreeItemClicked);
}

void MainWindow::onStartServer()
{
    GB28181Config config;
    config.serverId = m_serverIdEdit->text();
    config.serverIp = m_serverIpEdit->text();
    config.serverPort = m_serverPortSpin->value();

    if (m_server->start(config)) {
        m_startServerBtn->setEnabled(false);
        m_stopServerBtn->setEnabled(true);
        m_queryCatalogBtn->setEnabled(true);
        appendLog("服务器已启动");
    } else {
        QMessageBox::critical(this, "错误", "启动服务器失败");
    }
}

void MainWindow::onStopServer()
{
    m_server->stop();
    m_startServerBtn->setEnabled(true);
    m_stopServerBtn->setEnabled(false);
    m_queryCatalogBtn->setEnabled(false);
    appendLog("服务器已停止");
}

void MainWindow::onDeviceRegistered(const DeviceInfo &device)
{
    m_deviceManager->addDevice(device);
    appendLog(QString("设备注册: %1 (%2)").arg(device.name).arg(device.deviceId));
}

void MainWindow::onDeviceRemoved(const QString &deviceId)
{
    m_deviceManager->removeDevice(deviceId);
    m_channelManager->clearChannels(deviceId);
    appendLog(QString("设备注销: %1").arg(deviceId));
}

void MainWindow::onKeepaliveReceived(const QString &deviceId)
{
    m_deviceManager->updateDeviceKeepalive(deviceId);
}

void MainWindow::onCatalogReceived(const QString &deviceId, const QList<ChannelInfo> &channels)
{
    // 清除旧通道
    m_channelManager->clearChannels(deviceId);

    // 添加新通道
    for (const ChannelInfo &channel : channels) {
        m_channelManager->addChannel(channel);
    }

    updateChannelTree(deviceId);
    appendLog(QString("收到设备 %1 的 %2 个通道").arg(deviceId).arg(channels.size()));
}

void MainWindow::onQueryCatalog()
{
    if (m_selectedDeviceId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择设备");
        return;
    }

    m_server->sendCatalogQuery(m_selectedDeviceId);
    appendLog(QString("查询设备通道: %1").arg(m_selectedDeviceId));
}

void MainWindow::onStartPlay()
{
    if (m_selectedChannelId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择通道");
        return;
    }

    // 启动 RTP 接收器
    if (!m_rtpReceiver->start("0.0.0.0", 9000)) {
        QMessageBox::critical(this, "错误", "启动 RTP 接收器失败");
        return;
    }

    // 初始化视频解码器
    if (!m_videoDecoder->init(VideoCodec::H264)) {
        QMessageBox::critical(this, "错误", "初始化视频解码器失败");
        m_rtpReceiver->stop();
        return;
    }

    // 发送 INVITE
    m_server->sendInvite(m_selectedDeviceId, m_selectedChannelId,
                         m_serverIpEdit->text(), m_rtpReceiver->port());

    m_startPlayBtn->setEnabled(false);
    m_stopPlayBtn->setEnabled(true);

    appendLog(QString("开始播放通道: %1").arg(m_selectedChannelId));
}

void MainWindow::onStopPlay()
{
    if (m_currentSession.callId >= 0) {
        m_server->sendBye(m_currentSession.callId, m_currentSession.dialogId);
    }

    m_rtpReceiver->stop();
    m_videoDecoder->close();
    m_psParser->reset();

    m_startPlayBtn->setEnabled(true);
    m_stopPlayBtn->setEnabled(false);

    m_videoLabel->clear();
    m_videoLabel->setStyleSheet("QLabel { background-color: black; }");

    appendLog("停止播放");
}

void MainWindow::onMediaStreamReady(const SipSession &session)
{
    m_currentSession = session;
    appendLog(QString("媒体流就绪: %1:%2").arg(session.mediaInfo.ip).arg(session.mediaInfo.port));
}

void MainWindow::onVideoFrameDecoded(const QImage &image)
{
    m_videoLabel->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::onPTZUp()
{
    if (!m_selectedChannelId.isEmpty()) {
        m_server->sendPTZControl(m_selectedDeviceId, m_selectedChannelId, PTZCommand::Up, 50);
    }
}

void MainWindow::onPTZDown()
{
    if (!m_selectedChannelId.isEmpty()) {
        m_server->sendPTZControl(m_selectedDeviceId, m_selectedChannelId, PTZCommand::Down, 50);
    }
}

void MainWindow::onPTZLeft()
{
    if (!m_selectedChannelId.isEmpty()) {
        m_server->sendPTZControl(m_selectedDeviceId, m_selectedChannelId, PTZCommand::Left, 50);
    }
}

void MainWindow::onPTZRight()
{
    if (!m_selectedChannelId.isEmpty()) {
        m_server->sendPTZControl(m_selectedDeviceId, m_selectedChannelId, PTZCommand::Right, 50);
    }
}

void MainWindow::onPTZZoomIn()
{
    if (!m_selectedChannelId.isEmpty()) {
        m_server->sendPTZControl(m_selectedDeviceId, m_selectedChannelId, PTZCommand::ZoomIn, 50);
    }
}

void MainWindow::onPTZZoomOut()
{
    if (!m_selectedChannelId.isEmpty()) {
        m_server->sendPTZControl(m_selectedDeviceId, m_selectedChannelId, PTZCommand::ZoomOut, 50);
    }
}

void MainWindow::onPTZStop()
{
    if (!m_selectedChannelId.isEmpty()) {
        m_server->sendPTZControl(m_selectedDeviceId, m_selectedChannelId, PTZCommand::Stop, 0);
    }
}

void MainWindow::onDeviceTreeItemClicked(QTreeWidgetItem *item, int column)
{
    if (!item) {
        return;
    }

    QString type = item->data(0, Qt::UserRole).toString();

    if (type == "device") {
        m_selectedDeviceId = item->text(2);
        m_selectedChannelId.clear();
        m_startPlayBtn->setEnabled(false);
    } else if (type == "channel") {
        m_selectedChannelId = item->text(2);
        QTreeWidgetItem *parent = item->parent();
        if (parent) {
            m_selectedDeviceId = parent->text(2);
        }
        m_startPlayBtn->setEnabled(true);
    }
}

void MainWindow::updateDeviceTree()
{
    m_deviceTree->clear();

    QList<DeviceInfo> devices = m_deviceManager->getAllDevices();

    for (const DeviceInfo &device : devices) {
        QTreeWidgetItem *deviceItem = new QTreeWidgetItem(m_deviceTree);
        deviceItem->setText(0, device.name.isEmpty() ? device.deviceId : device.name);
        deviceItem->setText(1, device.status == DeviceStatus::Online ? "在线" : "离线");
        deviceItem->setText(2, device.deviceId);
        deviceItem->setData(0, Qt::UserRole, "device");

        // 添加通道
        QList<ChannelInfo> channels = m_channelManager->getDeviceChannels(device.deviceId);
        for (const ChannelInfo &channel : channels) {
            QTreeWidgetItem *channelItem = new QTreeWidgetItem(deviceItem);
            channelItem->setText(0, channel.name.isEmpty() ? channel.channelId : channel.name);
            channelItem->setText(1, channel.status == ChannelStatus::Online ? "在线" : "离线");
            channelItem->setText(2, channel.channelId);
            channelItem->setData(0, Qt::UserRole, "channel");
        }

        deviceItem->setExpanded(true);
    }
}

void MainWindow::updateChannelTree(const QString &deviceId)
{
    // 查找设备节点
    for (int i = 0; i < m_deviceTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *deviceItem = m_deviceTree->topLevelItem(i);
        if (deviceItem->text(2) == deviceId) {
            // 清除旧通道
            while (deviceItem->childCount() > 0) {
                delete deviceItem->takeChild(0);
            }

            // 添加新通道
            QList<ChannelInfo> channels = m_channelManager->getDeviceChannels(deviceId);
            for (const ChannelInfo &channel : channels) {
                QTreeWidgetItem *channelItem = new QTreeWidgetItem(deviceItem);
                channelItem->setText(0, channel.name.isEmpty() ? channel.channelId : channel.name);
                channelItem->setText(1, channel.status == ChannelStatus::Online ? "在线" : "离线");
                channelItem->setText(2, channel.channelId);
                channelItem->setData(0, Qt::UserRole, "channel");
            }

            deviceItem->setExpanded(true);
            break;
        }
    }
}

void MainWindow::appendLog(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    m_logText->append(QString("[%1] %2").arg(timestamp).arg(message));
}

} // namespace GB28181
