#ifndef __FFMPEG_DECODER__H__
#define __FFMPEG_DECODER__H__

/*************************************
** 使用 ffmpeg 实现硬解
*************************************/


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

#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <map>

#include "common.h"

class CHWDecoder{
public:
    CHWDecoder();
    ~CHWDecoder();
    // 获取ffmpeg错误信息
    static std::string getFFmpegErrMsg(int errnum);

public:
    // 是否在进行硬解解码
    bool getDecodeType() { return m_isHwDec; }
    // 设置、获取硬解类型
    void setHwDecType(std::string hwDecType) { m_hwDecType=hwDecType; }
    std::string getHwDecType() { return m_hwDecType; }

    /**
     * @fn	setVideoStreamType
     * @brief 设置原始数据类型.
     * @param[in] inputId 解码器类型
     * @param[out] 无
     * @return
     * @retval =1: 找到合适的硬解设备 =0: 未找到合适的硬解设备 <0: 出错
     */
    void setVideoStreamType(const enum AVCodecID inputId){m_inputCodecId=inputId;}

    /**
     * @fn	CodecInit
     * @brief 根据输入视频数据获取硬解码器.
     * @param[in] 无
     * @param[out] 无
     * @return
     * @retval =1: 找到合适的硬解设备 =0: 未找到合适的硬解设备 <0: 出错
     */
    int codecInit();

    /**
     * @fn	findSuitableHwDecoder
     * @brief 根据输入视频数据获取硬解码器.
     * @param[in] 无
     * @param[out] 无
     * @return
     * @retval =1: 找到合适的硬解设备 =0: 未找到合适的硬解设备 <0: 出错
     */
    int findSuitableHwDecoder();

    /**
     * @fn	doDecode
     * @brief 进行解码.
     * @param[in] pData 需要解析的数据
     * @param[in] dataLen 需要解析的数据长度
     * @param[out] result 解码后数据
     * @return
     * @retval =0: 成功 =1: 放入数据重试 2: 取流结尾 -1:失败
     */
    int doDecode(void* pData, unsigned int dataLen, DecodeData* result);

    /**
     * @fn	destroyResource
     * @brief 销毁资源.
     * @param[in] 无
     * @param[out] 无
     * @return
     * @retval 无
     */
    void destroyResource();

private:
    /**
     * @fn	checkHwDeviceSupport
     * @brief 指定硬解类型检查.
     * @param[in] hwTypeStr eg:cuda,d3d11va,vaapi...
     * @param[out] 无
     * @return
     * @retval =1: 找到合适的硬解设备 =0: 未找到合适的硬解设备 <0: 出错
     */
    bool checkHwDeviceSupport(std::string hwTypeStr);
    /**
     * @fn	getLocalSupported
     * @brief 获取本地支持硬解类型.
     * @param[in] 无
     * @param[out] std::vector<std::string>& supportHwVecOut [d3dva,cuda,vaapi...]
     * @return
     * @retval =0: 成功 <0: 失败
     */
    int getLocalSupported(std::vector<std::string>& supportHwVecOut);
    /**
     * @fn	hwDecodeInit
     * @brief 初始化硬件上下文.
     * @param[in] 无
     * @param[out] 无
     * @return
     * @retval =0: 成功 <0: 失败
     */
    int hwDecodeInit();

private:
    AVCodecContext* m_codecCtx;                 // 解码器上下文
    AVCodec* m_codec;                           // 解码器
    AVBufferRef* m_hwDeviceCtx;                 // 硬解码设备上下文
    enum AVHWDeviceType m_hwDevType;            // 硬解码类型
    std::vector<std::string> m_supportHwVec;    // 本机支持的硬解码类型
    enum AVCodecID m_inputCodecId;              // 输入数据编码类型
public:
    enum AVPixelFormat m_hwPixFmt;              // 硬解像素格式

private:
    bool m_isHwDec;             // 是否进行硬解
    std::string m_hwDecType;    // 硬解码类型 cuda,d3d11va,qsv....
};

#endif