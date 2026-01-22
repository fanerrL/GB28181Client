#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include <QObject>
#include <QImage>
#include "GB28181Types.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

namespace GB28181 {

class VideoDecoder : public QObject
{
    Q_OBJECT

public:
    explicit VideoDecoder(QObject *parent = nullptr);
    ~VideoDecoder();

    // 初始化解码器
    bool init(VideoCodec codec);
    void close();

    // 解码视频帧
    bool decode(const QByteArray &data);

signals:
    // 解码后的图像
    void frameDecoded(const QImage &image);

    // 错误
    void errorOccurred(const QString &error);

private:
    AVCodecContext *m_codecContext;
    AVFrame *m_frame;
    AVPacket *m_packet;
    SwsContext *m_swsContext;
    uint8_t *m_rgbBuffer;
    int m_rgbBufferSize;
};

} // namespace GB28181

#endif // VIDEO_DECODER_H
