#ifndef __MY_FFMPEG_DECODER__H__
#define __MY_FFMPEG_DECODER__H__


/*************************************
** 使用 ffmpeg 获取 url 码流
*************************************/

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavutil/avutil.h"
#include "libavutil/error.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/opt.h"
#include "libavutil/hwcontext.h"
#include "libavutil/imgutils.h"

#ifdef __cplusplus
}
#endif

#include "common.h"

// 解码视频流类
class CFFmpegDecoder{
public:
    CFFmpegDecoder();
    CFFmpegDecoder(std::string url);
    ~CFFmpegDecoder();
    // 获取ffmpeg错误信息
    static std::string getFFmpegErrMsg(int errnum);

public:
    // 设置/获取 url
    void setVideoUrl(std::string url){m_videoUrl=url;}
    std::string getVideoUrl(){return m_videoUrl;}

    /**
     * @fn	getStreamInfo
     * @brief 获取视频信息.
     * @param[in] streamInfo 输入流信息
     * @param[out] 无
     * @return
     * @retval 0-成功 !0-失败
     */
    int getStreamInfo(StreamInfo* streamInfo);

    /**
     * @fn	startWork
     * @brief 开始解复用数据.
     * @param[in] dataCallback 数据回调函数
     * @param[in] pUserData 回调函数用户自定义入参
     * @param[out] 无
     * @return
     * @retval 0-成功 !0-失败
     */
    int startWork(VideoCallBack dataCallback, void* pUserData);

    /**
     * @fn	stopWork
     * @brief 停止解复用数据.
     * @param[in] 无
     * @param[out] 无
     * @return
     * @retval 0-成功 !0-失败
     */
    int stopWork();

private:
    /**
     * @fn	doInit
     * @brief 初始化相关参数.
     * @param[in] 无
     * @param[out] 无
     * @return
     * @retval 0-成功 !0-失败
     */
    int doInit();

    /**
     * @fn	unInit
     * @brief 去初始化.
     * @param[in] 无
     * @param[out] 无
     * @return
     * @retval 0-成功 !0-失败
     */
    int unInit();

private:
    AVFormatContext* m_inputCtx;        // 输入上下文
#if 0
    // 目前获取编码数据未使用
    AVCodecContext* m_codecCtxV;        // 视频解码器上下文
    AVCodecContext* m_codecCtxA;        // 音频解码器上下文
    AVCodec* m_codecV;                  // 视频解码器
    AVCodec* m_codecA;                  // 音频解码器
#endif
    AVStream* m_video;                  // video Stream
    AVStream* m_audio;                  // audio Stream
    int m_vStreamIndex;                 // video index
    int m_aStreamIndex;                 // audio index
    StreamInfo m_streamInfo;           // 流信息
    std::string m_videoUrl;             // 视频 url
    bool isInit;                        // 是否进行了初始化

    bool isStop;                        // 是否停止
    bool isDealDataOver;                // 是否处理完数据
};

#endif