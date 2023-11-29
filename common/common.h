#ifndef __HW_DECODE_H__
#define __HW_DECODE_H__


#ifdef __cplusplus
extern "C" {
#endif


typedef enum{
    ERR_HW_UNKNOW = -1,     // 未知
    ERR_HW_SUCCESS = 0,     // 成功
    ERR_HW_PARAM = 1,       // 参数异常
    ERR_HW_MEM = 2,         // 内存错误
    ERR_HW_FFMPEG = 3,      // ffmpeg操作失败
    ERR_HW_MAX = 100
}E_HwErr;

typedef enum{
    STREAM_TYPE_UNKNOW = 0,
    STREAM_TYPE_URL = 1,        // rtsp,rtmp..或者本地文件
    STREAM_TYPE_BSTAR = 2,      // bstp数据数据-逐帧给数据
    STREAM_TYPE_TS = 3,         // TS数据数据
    STREAM_TYPE_SS = 4,         // SS数据数据
    STREAM_TYPE_MAX = 20
}EStreamType;

// tcp方式获取视频数据 - TS获取
typedef struct{
    char dstIp[16];             // 目标地址
    int dstPort;                // 目标端口
    unsigned int streamId;      // 流id
}TS;

// Ticket - SServer取流
typedef struct{
    int          dataType; // bscp 协议 1: 透明传输 2:传输 3:语音 4: 下载 5: 回放 6:图片下载 7:图片回放 8: 报警信息
    unsigned int streamId;
    unsigned int sessionId;
}Ticket;

// 原始数据帧类型
typedef struct{
    int codecId;        // 编码类型 h264,h265
}VideoSrc;

// 原始数据
typedef struct{
    void* data;         // 原始数据
    int size;           // 原始数据大小
}SrcData;

// 流描述信息
typedef struct{
    int streamType;     // 数据信息EStreamType
    union {
        char url[256];
        TS ts;
        Ticket ticket;
        VideoSrc videoType;
    }overLoader;
}StrmInputDesc;

typedef void* HW_HADNLE;
typedef void* DECODER_HANDLE;


// 数据类型
enum {
    DATA_OF_UNKNOW = -1,
    DATA_OF_AUDIO = 1,
    DATA_OF_VIDEO = 2,
    DATA_OF_MAX = 10
};

// 解复用后的编码数据
typedef struct{
    int dataType;       // 数据类型 DATA_OF_AUDIO、DATA_OF_VIDEO
    void* data;         // 数据
    int dataLen;        // 数据长度
}StreamData;

// 数据回调函数 - 目前获取解复用后的数据
typedef int VideoCallBack(StreamData* pStreamData, void* pUserData);

// 视频信息
class CVideoInfo{
public:
    int videoType;      // 视频类型 AVCodecId
    int pixFmt;         // 像素格式 AVPixelFormat
    int width;          // 宽
    int height;         // 高
};

// 音频信息
typedef struct{
    int audioType;      // 音频类型 AVSampleFormat
    int sampleRate;
    int channels;
    int bitRate;
}AudioInfo;

// 视频信息
typedef struct {
    int videoType;      // 视频类型 AVCodecId
    int pixFmt;         // 像素格式 AVPixelFormat
    int width;          // 宽
    int height;         // 高
}VideoInfo;

// 视频流信息
typedef struct {
    unsigned char streamSum;       // DATA_OF_AUDIO、DATA_OF_VIDEO 的位运算
    VideoInfo videoInfo;           // 视频信息 streamSum & DATA_OF_VIDEO == 1
    AudioInfo audioInfo;           // 音频信息 streamSum & DATA_OF_AUDIO == 1
}StreamInfo;

// 解复用后的编码数据
typedef struct{
    int type;           // 类型
    int pixFmt;         // 像素格式
    void* data;         // 数据
    int size;           // 数据长度
}EncodeData;

// 解码后原始视频数据
typedef struct {
    int pixFmt;         // 像素格式
    void* data;         // 解码后数据
    int size;           // 解码后数据大小
    int width;          // 宽高
    int height;
}DecodeData;

#ifdef __cplusplus
}
#endif


#endif