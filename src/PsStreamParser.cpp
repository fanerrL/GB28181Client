#include "PsStreamParser.h"
#include <QDebug>

namespace GB28181 {

PsStreamParser::PsStreamParser(QObject *parent)
    : QObject(parent)
    , m_videoCodec(VideoCodec::H264)
    , m_hasSystemHeader(false)
{
}

PsStreamParser::~PsStreamParser()
{
}

void PsStreamParser::parse(const QByteArray &data)
{
    int offset = 0;

    while (offset < data.size() - 4) {
        // 查找起始码 0x000001
        if ((unsigned char)data[offset] != 0x00 ||
            (unsigned char)data[offset+1] != 0x00 ||
            (unsigned char)data[offset+2] != 0x01) {
            offset++;
            continue;
        }

        unsigned char streamId = (unsigned char)data[offset+3];

        if (streamId == 0xBA) {
            // PS 包头
            if (!parsePsPackHeader(data, offset)) {
                break;
            }
        } else if (streamId == 0xBB) {
            // 系统头
            if (!parseSystemHeader(data, offset)) {
                break;
            }
        } else if (streamId == 0xBC) {
            // 节目流映射
            if (!parseProgramStreamMap(data, offset)) {
                break;
            }
        } else if ((streamId >= 0xC0 && streamId <= 0xDF) ||
                   (streamId >= 0xE0 && streamId <= 0xEF) ||
                   streamId == 0xBD) {
            // PES 包
            if (!parsePesPacket(data, offset)) {
                break;
            }
        } else {
            offset++;
        }
    }
}

void PsStreamParser::reset()
{
    m_videoBuffer.clear();
    m_audioBuffer.clear();
    m_hasSystemHeader = false;
}

bool PsStreamParser::parsePsPackHeader(const QByteArray &data, int &offset)
{
    // PS 包头固定14字节
    if (offset + 14 > data.size()) {
        return false;
    }

    // 跳过 PS 包头
    offset += 14;

    // 检查填充长度
    if (offset < data.size()) {
        unsigned char stuffingLen = (unsigned char)data[offset-1] & 0x07;
        offset += stuffingLen;
    }

    return true;
}

bool PsStreamParser::parseSystemHeader(const QByteArray &data, int &offset)
{
    if (offset + 6 > data.size()) {
        return false;
    }

    // 读取长度
    int length = ((unsigned char)data[offset+4] << 8) | (unsigned char)data[offset+5];

    if (offset + 6 + length > data.size()) {
        return false;
    }

    // 跳过系统头
    offset += 6 + length;
    m_hasSystemHeader = true;

    return true;
}

bool PsStreamParser::parseProgramStreamMap(const QByteArray &data, int &offset)
{
    if (offset + 6 > data.size()) {
        return false;
    }

    // 读取长度
    int length = ((unsigned char)data[offset+4] << 8) | (unsigned char)data[offset+5];

    if (offset + 6 + length > data.size()) {
        return false;
    }

    // 跳过节目流映射
    offset += 6 + length;

    return true;
}

bool PsStreamParser::parsePesPacket(const QByteArray &data, int &offset)
{
    if (offset + 9 > data.size()) {
        return false;
    }

    unsigned char streamId = (unsigned char)data[offset+3];

    // 读取 PES 包长度
    int pesLength = ((unsigned char)data[offset+4] << 8) | (unsigned char)data[offset+5];

    if (pesLength == 0) {
        // 长度为0表示不限制长度，读取到下一个起始码
        pesLength = data.size() - offset - 6;
    }

    if (offset + 6 + pesLength > data.size()) {
        return false;
    }

    // PES 头标志
    unsigned char pesHeaderDataLen = (unsigned char)data[offset+8];

    int pesHeaderLen = 9 + pesHeaderDataLen;

    if (offset + pesHeaderLen > data.size()) {
        return false;
    }

    // 提取 PES payload
    int payloadLen = pesLength - 3 - pesHeaderDataLen;
    if (payloadLen <= 0) {
        offset += 6 + pesLength;
        return true;
    }

    QByteArray payload = data.mid(offset + pesHeaderLen, payloadLen);

    // 判断流类型
    if (streamId >= 0xE0 && streamId <= 0xEF) {
        // 视频流
        extractH264Frame(payload);
    } else if (streamId >= 0xC0 && streamId <= 0xDF) {
        // 音频流
        emit audioFrame(payload);
    }

    offset += 6 + pesLength;

    return true;
}

void PsStreamParser::extractH264Frame(const QByteArray &pesPayload)
{
    m_videoBuffer.append(pesPayload);

    // 查找 H264 NALU 起始码
    int pos = 0;
    while (pos < m_videoBuffer.size() - 4) {
        int startCodeLen = 0;

        // 查找 0x00000001 或 0x000001
        if ((unsigned char)m_videoBuffer[pos] == 0x00 &&
            (unsigned char)m_videoBuffer[pos+1] == 0x00) {

            if ((unsigned char)m_videoBuffer[pos+2] == 0x01) {
                startCodeLen = 3;
            } else if ((unsigned char)m_videoBuffer[pos+2] == 0x00 &&
                       (unsigned char)m_videoBuffer[pos+3] == 0x01) {
                startCodeLen = 4;
            }
        }

        if (startCodeLen > 0) {
            // 找到起始码，查找下一个起始码
            int nextPos = findStartCode(m_videoBuffer, pos + startCodeLen);

            if (nextPos > 0) {
                // 提取完整的 NALU
                QByteArray nalu = m_videoBuffer.mid(pos, nextPos - pos);

                // 检查是否为关键帧
                bool isKeyFrame = isH264KeyFrame(nalu);

                emit videoFrame(nalu, m_videoCodec, isKeyFrame);

                // 移除已处理的数据
                m_videoBuffer = m_videoBuffer.mid(nextPos);
                pos = 0;
            } else {
                // 没有找到下一个起始码，等待更多数据
                break;
            }
        } else {
            pos++;
        }
    }

    // 限制缓冲区大小
    if (m_videoBuffer.size() > 1024 * 1024) {
        m_videoBuffer = m_videoBuffer.right(512 * 1024);
    }
}

bool PsStreamParser::isH264KeyFrame(const QByteArray &data)
{
    // 查找 NALU 类型
    for (int i = 0; i < data.size() - 4; ++i) {
        if ((unsigned char)data[i] == 0x00 &&
            (unsigned char)data[i+1] == 0x00) {

            int startCodeLen = 0;
            if ((unsigned char)data[i+2] == 0x01) {
                startCodeLen = 3;
            } else if ((unsigned char)data[i+2] == 0x00 &&
                       (unsigned char)data[i+3] == 0x01) {
                startCodeLen = 4;
            }

            if (startCodeLen > 0 && i + startCodeLen < data.size()) {
                unsigned char naluType = (unsigned char)data[i + startCodeLen] & 0x1F;

                // NALU 类型 5 为 IDR 帧（关键帧）
                // NALU 类型 7 为 SPS
                // NALU 类型 8 为 PPS
                if (naluType == 5 || naluType == 7 || naluType == 8) {
                    return true;
                }
            }
        }
    }

    return false;
}

int PsStreamParser::findStartCode(const QByteArray &data, int offset)
{
    for (int i = offset; i < data.size() - 3; ++i) {
        if ((unsigned char)data[i] == 0x00 &&
            (unsigned char)data[i+1] == 0x00) {

            if ((unsigned char)data[i+2] == 0x01) {
                return i;
            } else if (i < data.size() - 4 &&
                       (unsigned char)data[i+2] == 0x00 &&
                       (unsigned char)data[i+3] == 0x01) {
                return i;
            }
        }
    }

    return -1;
}

} // namespace GB28181
