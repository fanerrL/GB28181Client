cmake_minimum_required(VERSION 3.16)
project(GB28181Client VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 配置
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Multimedia MultimediaWidgets)

# FFmpeg 配置
set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/3rdparty/ffmpeg" CACHE PATH "FFmpeg directory")
find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h PATHS ${FFMPEG_DIR}/include)
find_library(AVCODEC_LIBRARY avcodec PATHS ${FFMPEG_DIR}/lib)
find_library(AVFORMAT_LIBRARY avformat PATHS ${FFMPEG_DIR}/lib)
find_library(AVUTIL_LIBRARY avutil PATHS ${FFMPEG_DIR}/lib)
find_library(SWSCALE_LIBRARY swscale PATHS ${FFMPEG_DIR}/lib)
find_library(SWRESAMPLE_LIBRARY swresample PATHS ${FFMPEG_DIR}/lib)

# eXosip2 配置
set(EXOSIP2_DIR "${CMAKE_SOURCE_DIR}/3rdparty/exosip2" CACHE PATH "eXosip2 directory")
find_path(EXOSIP2_INCLUDE_DIR eXosip2/eXosip.h PATHS ${EXOSIP2_DIR}/include)
find_library(EXOSIP2_LIBRARY eXosip2 PATHS ${EXOSIP2_DIR}/lib)
find_library(OSIP2_LIBRARY osip2 PATHS ${EXOSIP2_DIR}/lib)
find_library(OSIPPARSER2_LIBRARY osipparser2 PATHS ${EXOSIP2_DIR}/lib)

# 源文件
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/GB28181Server.cpp
    src/DeviceManager.cpp
    src/ChannelManager.cpp
    src/SipMessageHandler.cpp
    src/RtpReceiver.cpp
    src/PsStreamParser.cpp
    src/VideoPlayer.cpp
    src/VideoDecoder.cpp
    src/PTZController.cpp
)

# 头文件
set(HEADERS
    include/MainWindow.h
    include/GB28181Server.h
    include/DeviceManager.h
    include/ChannelManager.h
    include/SipMessageHandler.h
    include/RtpReceiver.h
    include/PsStreamParser.h
    include/VideoPlayer.h
    include/VideoDecoder.h
    include/PTZController.h
    include/GB28181Types.h
)

# UI 文件
set(UI_FILES
    ui/MainWindow.ui
)

# 资源文件
set(RESOURCES
    resources/resources.qrc
)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${FFMPEG_INCLUDE_DIR}
    ${EXOSIP2_INCLUDE_DIR}
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCES}
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    ${AVCODEC_LIBRARY}
    ${AVFORMAT_LIBRARY}
    ${AVUTIL_LIBRARY}
    ${SWSCALE_LIBRARY}
    ${SWRESAMPLE_LIBRARY}
    ${EXOSIP2_LIBRARY}
    ${OSIP2_LIBRARY}
    ${OSIPPARSER2_LIBRARY}
)

# Windows 特定配置
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32 iphlpapi)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
