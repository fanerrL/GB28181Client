cmake_minimum_required(VERSION 3.16)
project(GB28181Client VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 配置（必需）
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

# FFmpeg 配置（可选）
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(FFMPEG QUIET
        libavcodec
        libavformat
        libavutil
        libswscale
        libswresample
    )
endif()

# 如果 pkg-config 找不到，尝试手动查找
if(NOT FFMPEG_FOUND)
    find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
    find_library(AVCODEC_LIBRARY avcodec)
    find_library(AVFORMAT_LIBRARY avformat)
    find_library(AVUTIL_LIBRARY avutil)
    find_library(SWSCALE_LIBRARY swscale)
    find_library(SWRESAMPLE_LIBRARY swresample)

    if(AVCODEC_INCLUDE_DIR AND AVCODEC_LIBRARY)
        set(FFMPEG_FOUND TRUE)
        set(FFMPEG_INCLUDE_DIRS ${AVCODEC_INCLUDE_DIR})
        set(FFMPEG_LIBRARIES
            ${AVCODEC_LIBRARY}
            ${AVFORMAT_LIBRARY}
            ${AVUTIL_LIBRARY}
            ${SWSCALE_LIBRARY}
            ${SWRESAMPLE_LIBRARY}
        )
    endif()
endif()

# eXosip2 配置（可选）
find_path(EXOSIP2_INCLUDE_DIR eXosip2/eXosip.h)
find_library(EXOSIP2_LIBRARY eXosip2)
find_library(OSIP2_LIBRARY osip2)
find_library(OSIPPARSER2_LIBRARY osipparser2)

if(EXOSIP2_INCLUDE_DIR AND EXOSIP2_LIBRARY)
    set(EXOSIP2_FOUND TRUE)
    set(EXOSIP2_INCLUDE_DIRS ${EXOSIP2_INCLUDE_DIR})
    set(EXOSIP2_LIBRARIES
        ${EXOSIP2_LIBRARY}
        ${OSIP2_LIBRARY}
        ${OSIPPARSER2_LIBRARY}
    )
endif()

# 源文件
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/GB28181Server.cpp
    src/DeviceManager.cpp
    src/ChannelManager.cpp
    src/RtpReceiver.cpp
    src/PsStreamParser.cpp
    src/VideoDecoder.cpp
)

# 头文件
set(HEADERS
    include/MainWindow.h
    include/GB28181Server.h
    include/DeviceManager.h
    include/ChannelManager.h
    include/RtpReceiver.h
    include/PsStreamParser.h
    include/VideoDecoder.h
    include/GB28181Types.h
)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# 链接 Qt（必需）
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# 链接 FFmpeg（如果找到）
if(FFMPEG_FOUND)
    message(STATUS "FFmpeg found, enabling video decoding")
    target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBRARIES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_FFMPEG)
else()
    message(WARNING "FFmpeg not found, video decoding will be disabled")
endif()

# 链接 eXosip2（如果找到）
if(EXOSIP2_FOUND)
    message(STATUS "eXosip2 found, enabling SIP functionality")
    target_include_directories(${PROJECT_NAME} PRIVATE ${EXOSIP2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${EXOSIP2_LIBRARIES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_EXOSIP2)
else()
    message(WARNING "eXosip2 not found, SIP functionality will be disabled")
endif()

# Windows 特定配置
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32 iphlpapi)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# 打印配置信息
message(STATUS "=== GB28181Client Configuration ===")
message(STATUS "Qt6 found: YES")
message(STATUS "FFmpeg found: ${FFMPEG_FOUND}")
message(STATUS "eXosip2 found: ${EXOSIP2_FOUND}")
message(STATUS "===================================")
