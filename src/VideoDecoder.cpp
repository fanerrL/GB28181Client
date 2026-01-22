#include "VideoDecoder.h"
#include <QDebug>

namespace GB28181 {

VideoDecoder::VideoDecoder(QObject *parent)
    : QObject(parent)
    , m_codecContext(nullptr)
    , m_frame(nullptr)
    , m_packet(nullptr)
    , m_swsContext(nullptr)
    , m_rgbBuffer(nullptr)
    , m_rgbBufferSize(0)
{
}

VideoDecoder::~VideoDecoder()
{
    close();
}

bool VideoDecoder::init(VideoCodec codec)
{
    close();

    // 查找解码器
    AVCodecID codecId = AV_CODEC_ID_NONE;
    switch (codec) {
    case VideoCodec::H264:
        codecId = AV_CODEC_ID_H264;
        break;
    case VideoCodec::H265:
        codecId = AV_CODEC_ID_HEVC;
        break;
    default:
        emit errorOccurred("Unsupported codec");
        return false;
    }

    const AVCodec *avCodec = avcodec_find_decoder(codecId);
    if (!avCodec) {
        emit errorOccurred("Failed to find decoder");
        return false;
    }

    // 创建解码器上下文
    m_codecContext = avcodec_alloc_context3(avCodec);
    if (!m_codecContext) {
        emit errorOccurred("Failed to allocate codec context");
        return false;
    }

    // 打开解码器
    if (avcodec_open2(m_codecContext, avCodec, nullptr) < 0) {
        emit errorOccurred("Failed to open codec");
        avcodec_free_context(&m_codecContext);
        return false;
    }

    // 分配帧和包
    m_frame = av_frame_alloc();
    m_packet = av_packet_alloc();

    if (!m_frame || !m_packet) {
        emit errorOccurred("Failed to allocate frame or packet");
        close();
        return false;
    }

    qDebug() << "Video decoder initialized:" << avCodec->name;

    return true;
}

void VideoDecoder::close()
{
    if (m_swsContext) {
        sws_freeContext(m_swsContext);
        m_swsContext = nullptr;
    }

    if (m_rgbBuffer) {
        av_free(m_rgbBuffer);
        m_rgbBuffer = nullptr;
        m_rgbBufferSize = 0;
    }

    if (m_packet) {
        av_packet_free(&m_packet);
    }

    if (m_frame) {
        av_frame_free(&m_frame);
    }

    if (m_codecContext) {
        avcodec_free_context(&m_codecContext);
    }
}

bool VideoDecoder::decode(const QByteArray &data)
{
    if (!m_codecContext || !m_frame || !m_packet) {
        return false;
    }

    // 设置包数据
    m_packet->data = (uint8_t*)data.constData();
    m_packet->size = data.size();

    // 发送包到解码器
    int ret = avcodec_send_packet(m_codecContext, m_packet);
    if (ret < 0) {
        qDebug() << "Error sending packet to decoder:" << ret;
        return false;
    }

    // 接收解码后的帧
    while (ret >= 0) {
        ret = avcodec_receive_frame(m_codecContext, m_frame);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            qDebug() << "Error receiving frame from decoder:" << ret;
            return false;
        }

        // 转换为 RGB 格式
        if (!m_swsContext || m_frame->width != m_codecContext->width ||
            m_frame->height != m_codecContext->height) {

            if (m_swsContext) {
                sws_freeContext(m_swsContext);
            }

            m_swsContext = sws_getContext(
                m_frame->width, m_frame->height, (AVPixelFormat)m_frame->format,
                m_frame->width, m_frame->height, AV_PIX_FMT_RGB24,
                SWS_BILINEAR, nullptr, nullptr, nullptr);

            if (!m_swsContext) {
                emit errorOccurred("Failed to create sws context");
                return false;
            }

            // 重新分配 RGB 缓冲区
            int newSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24,
                                                    m_frame->width,
                                                    m_frame->height, 1);
            if (newSize != m_rgbBufferSize) {
                if (m_rgbBuffer) {
                    av_free(m_rgbBuffer);
                }
                m_rgbBuffer = (uint8_t*)av_malloc(newSize);
                m_rgbBufferSize = newSize;
            }
        }

        // 转换图像
        uint8_t *dstData[4] = {m_rgbBuffer, nullptr, nullptr, nullptr};
        int dstLinesize[4] = {m_frame->width * 3, 0, 0, 0};

        sws_scale(m_swsContext,
                  m_frame->data, m_frame->linesize, 0, m_frame->height,
                  dstData, dstLinesize);

        // 创建 QImage
        QImage image(m_rgbBuffer, m_frame->width, m_frame->height,
                     m_frame->width * 3, QImage::Format_RGB888);

        emit frameDecoded(image.copy());
    }

    return true;
}

} // namespace GB28181
