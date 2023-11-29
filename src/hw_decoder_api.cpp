#include "hw_decoder_api.h"
#include "utils.h"
#include "HwDecoder.h"
#include "FFmpegDecoder.h"

#include <map>
#include <string>
using namespace std;

int DEBUG_WRITE_FILE = 0;       // 数据写文件
string g_userCustomHwDevice=""; //用户自定义硬解类型
int g_ffmpegLogLevel = AV_LOG_INFO;

#define CONFIG_FILE "./config/config.ini"   // 配置文件

typedef map<string, string> ConfMap;
typedef map<string, string>::iterator ConfMapIter;
ConfMap g_confMap;  // 配置文件数据map

const char* HW_SET_FFMPEG_LOG_LEVEL = "ffmpeg_log_level";
const char* HW_IS_DEBUG_WRITE_FILE = "write_video_file";
const char* HW_SET_FFMPEG_HWDEVICE = "hardware_decode_type";

typedef std::map<E_HwErr, std::string> HWErrMap;
HWErrMap g_errMap = {
    {ERR_HW_UNKNOW, "UnKnow Err"},
    {ERR_HW_SUCCESS, "Success"},
    {ERR_HW_PARAM, "Invalid Param"},
    {ERR_HW_MEM, "Memory Err"},
    {ERR_HW_MAX, "Err Max"}
};

// 获取配置中ffmpeg日志级别
int GetFFmpegLogLevelFromConf()
{
    int ret = (int)AV_LOG_INFO;
    ConfMapIter iter = g_confMap.find(HW_SET_FFMPEG_LOG_LEVEL);
    if (iter != g_confMap.end() && !iter->second.empty())
    {
        if (iter->second == "INFO")
            ret = (int)AV_LOG_INFO;
        else if (iter->second == "WARN")
            ret = (int)AV_LOG_WARNING;
        else if (iter->second == "ERROR")
            ret = (int)AV_LOG_ERROR;
        else if (iter->second == "DEBUG")
            ret = (int)AV_LOG_DEBUG;
        else if (iter->second == "TRACE")
            ret = (int)AV_LOG_TRACE;
        else
            ret = (int)AV_LOG_INFO;
    }

    return ret;
}

// 获取配置中是否解码日志写文件
int GetIsWriteDebugFileFromConf()
{
    int ret = 0;
    ConfMapIter iter = g_confMap.find(HW_IS_DEBUG_WRITE_FILE);
    if (iter != g_confMap.end() && !iter->second.empty())
    {
        atoi(iter->second.c_str()) != 0 ? ret = 1 : ret = 0;
    }

    return ret;
}

// 获取配置中需要使用的硬解
string GetCustomHwDeviceFromConf()
{
    string customHwDev;
    ConfMapIter iter = g_confMap.find(HW_SET_FFMPEG_HWDEVICE);
    if (iter != g_confMap.end() && !iter->second.empty())
    {
        customHwDev = iter->second;
    }

    return customHwDev;
}

// 读取配置文件./config/config.ini
int bstar_hwdec_read_config(const char* configFile)
{
    FILE* confFile = fopen(configFile, "r");
    if (confFile == nullptr){
        MLOG_ERROR("Read Config File Err![%s] ret[%d]-[%s]", configFile ? configFile : "", errno, strerror(errno));
        return 0;
    }

    string tmpKey;
    while(!feof(confFile))
    {
        char bufLine[1024]={0};
        fgets(bufLine, sizeof(bufLine), confFile);
        if(bufLine[0] != 0)
        {
            int strLen = (int)strlen(bufLine);
            int tmp = 0;
            while(tmp < strLen)
            {
                if(bufLine[tmp] == ' ' || bufLine[tmp] == '\t')
                {
                    tmp++;
                }
                else
                {
                    break;
                }
            }

            if (tmp < strLen)
            {
                // 注释
                if(bufLine[tmp] == '#')
                    continue;
                // 标题
                if(bufLine[tmp] == '[')
                {
                    int tmpEnd = strLen - 1;
                    while (tmpEnd >= tmp)
                    {
                        if (bufLine[tmpEnd] == '\r')
                            bufLine[tmpEnd] = '\0';
                        else if (bufLine[tmpEnd] == '\n')
                            bufLine[tmpEnd] = '\0';
                        else if (bufLine[tmpEnd] == ' ')
                            bufLine[tmpEnd] = '\0';
                        else if (bufLine[tmpEnd] == ']')
                            bufLine[tmpEnd] = '\0';
                        else
                            break;
                        tmpEnd--;
                    }

                    g_confMap.insert(make_pair(bufLine+tmp+1, ""));
                    tmpKey = bufLine+tmp+1;
                }
                else
                {
                    int tmpEnd = strLen - 1;
                    while (tmpEnd >= tmp)
                    {
                        if (bufLine[tmpEnd] == '\r')
                            bufLine[tmpEnd] = '\0';
                        else if (bufLine[tmpEnd] == '\n')
                            bufLine[tmpEnd] = '\0';
                        else
                            break;
                        tmpEnd--;
                    }

                    g_confMap[tmpKey] == "" ? g_confMap[tmpKey] = bufLine + tmp : g_confMap[tmpKey];
                }
            }
        }
    }


    MLOG_INFO("@@ Config:");
    if (g_confMap.size() == 0)
    {
        MLOG_INFO("\t\t empty!");
    }
    else
    {
        for(auto var:g_confMap)
        {
            MLOG_INFO("\t\t key:[%s] value[%s]", var.first.c_str(), var.second.c_str());
        }
    }

    return 0;
}

/**
 * @fn	bstar_hwdec_doinit
 * @brief 硬解库初始化.
 * @param[in] 无
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
int bstar_hwdec_doinit(const char* configFile)
{
    bstar_hwdec_read_config(configFile);

    g_userCustomHwDevice = GetCustomHwDeviceFromConf();
    g_ffmpegLogLevel = GetFFmpegLogLevelFromConf();
    DEBUG_WRITE_FILE = GetIsWriteDebugFileFromConf();
    MLOG_ERROR("Custom HWDevice[%s] ffmpeg LogLevel[%d] isWriteDebugFile[%d]", g_userCustomHwDevice.c_str(), g_ffmpegLogLevel, DEBUG_WRITE_FILE);

    int ret = avformat_network_init();
    if (ret != 0)
    {
        MLOG_INFO("avformat_network_init err!! [%s]", CHWDecoder::getFFmpegErrMsg(ret).c_str());
        return -1;
    }
    // 设置ffmpeg 日志级别
    av_log_set_level(g_ffmpegLogLevel);
    return 0;
}

/**
 * @fn	bstar_hwdec_get_instance
 * @brief 获取解码实例.
 * @param[in] 无
 * @param[out] 无
 * @return
 * @retval NULL-成功 !NULL-失败
 */
HW_HADNLE bstar_hwdec_get_instance()
{
    HW_HADNLE retHandle = nullptr;
    retHandle = new CHWDecoder;
    if (retHandle == nullptr)
    {
        MLOG_ERROR("Malloc CFFmpegDecoder Err!!");
    }
    return retHandle;
}

/**
 * @fn	bstar_hwdec_set_input_desc
 * @brief 设置需要进行硬解的原始数据信息.
 * @param[in] handle  硬解实例
 * @param[in] inputDesc 输入信息描述
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
int bstar_hwdec_set_input_desc(HW_HADNLE handle, StrmInputDesc* inputDesc)
{
    if (inputDesc == nullptr)
    {
        return ERR_HW_PARAM;
    }

    int ret = ERR_HW_UNKNOW;
    // url 方式
    if (inputDesc->streamType == STREAM_TYPE_URL)
    {

    }
    // TS 取流
    else if (inputDesc->streamType == STREAM_TYPE_TS)
    {

    }
    // SS 取流
    else if (inputDesc->streamType == STREAM_TYPE_SS)
    {

    }
    // bsr 数据逐帧
    else if (inputDesc->streamType == STREAM_TYPE_BSTAR)
    {
        enum AVCodecID codecId = (enum AVCodecID)inputDesc->overLoader.videoType.codecId;
        ((CHWDecoder*)handle)->setVideoStreamType(codecId);
        ret = ((CHWDecoder*)handle)->findSuitableHwDecoder();
        if (ret != 0)
        {
            MLOG_ERROR("findSuitableHwDecoder Err!!");
            ret = ERR_HW_FFMPEG;
        }
    }
    else
    {
        return ERR_HW_PARAM;
    }

    return ret;
}

/**
 * @fn	bstar_hwdec_get_instance
 * @brief 设置硬解类型.
 * @param[in] handle  硬解实例
 * @param[in] hwType  硬解类型,d3d11va,qsv,vaapi等,如果支持则使用设置的硬解类型,如果不支持则进行软解
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
int bstar_hwdec_set_type(HW_HADNLE handle, const char* hwType)
{
    if (!handle || !hwType || hwType[0]==0)
    {
        MLOG_ERROR("Invalid Handle[%p] hwType[%p][%s]", handle, hwType, hwType==nullptr?"":hwType);
        return ERR_HW_PARAM;
    }
    ((CHWDecoder*)handle)->setHwDecType(hwType);
    return 0;
}

/**
 * @fn	bstar_hwdec_push_data
 * @brief 推送需要解码的视频数据.
 * @param[in] handle 硬解实例
 * @param[in] srcData 输入是视频编码数据(h264,h265等数据包)
 * @param[out] result 解码后的原始视频数据
 * @return
 * @retval =0: 成功 =1: 放入数据重试 2: 取流结尾 -1:失败
 */
int bstar_hwdec_push_data(HW_HADNLE handle, SrcData *srcData, DecodeData* result)
{
    if (!handle || !srcData || !result)
    {
        MLOG_ERROR("Invalid Handle[%p] srcData[%p] result[%p]", handle, srcData, result);
        return ERR_HW_PARAM;
    }

    int ret = ERR_HW_UNKNOW;
    ret = ((CHWDecoder*)handle)->doDecode(srcData->data, srcData->size, result);
    if (ret == -1) {
        ret = ERR_HW_FFMPEG;
        MLOG_ERROR("DoDecode Err!!");
    } else if (ret == 1){
        MLOG_WARN("Push Data Again!!");
    } else if (ret == 2){
        MLOG_WARN("Data Of Eof!!");
    }
    return ret;
}

/**
 * @fn	bstar_hwdec_del_instance
 * @brief 删除解码实例.
 * @param[in] handle
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
int bstar_hwdec_del_instance(HW_HADNLE handle)
{
    if (!handle)
    {
        MLOG_ERROR("Invalid Handle[%p]", handle);
        return ERR_HW_PARAM;
    }
    ((CHWDecoder*)handle)->destroyResource();
    return 0;
}


/**
 * @fn	bstar_decoder_get_instance
 * @brief 根据输入url获取解码实例.
 * @param[in] fileUrl 输入流信息,rtsp,rtmp,本地文件
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
DECODER_HANDLE bstar_decoder_get_instance(const char* fileUrl)
{
    if (fileUrl==nullptr || fileUrl[0]=='\0')
    {
        MLOG_ERROR("Invalid Input Param!!");
        return nullptr;
    }
    DECODER_HANDLE retDecoder = (void*) new CFFmpegDecoder(fileUrl);
    if (retDecoder == nullptr)
    {
        MLOG_ERROR("Malloc Decoder Err!!");
        return nullptr;
    }

    return retDecoder;
}

/**
 * @fn	bstar_decoder_get_stream_info
 * @brief 获取输入url的流信息.
 * @param[in] handle 解复用实例
 * @param[out] pStreamInfo 流信息
 * @return
 * @retval 0-成功 !0-失败
 */
int bstar_decoder_get_stream_info(DECODER_HANDLE handle, StreamInfo* pStreamInfo)
{
    if (nullptr == handle || nullptr == pStreamInfo)
    {
        MLOG_ERROR("Invalid Input Param!!");
        return ERR_HW_PARAM;
    }
    CFFmpegDecoder* decoder = (CFFmpegDecoder*)handle;
    int ret = decoder->getStreamInfo(pStreamInfo);
    if (ret != 0)
    {
        MLOG_ERROR("Get Stream Info Err!!");
        return ERR_HW_FFMPEG;
    }

    MLOG_INFO("FFMPEG LOG Level:[%d]", av_log_get_level());

    return 0;
}

/**
 * @fn	bstar_decoder_start_decode
 * @brief 开始解复用.
 * @param[in] handle 解复用实例
 * @param[in] dataCallBack 获取解复用数据回调,用户回调接口不能阻塞时间太长,最好将数据放入队列处理
 * @param[in] pUserData 回调函数自定义入参
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
int bstar_decoder_start_decode(DECODER_HANDLE handle, VideoCallBack dataCallBack, void* pUserData)
{
    if (nullptr == handle)
    {
        MLOG_ERROR("Invalid Input Param!!");
        return ERR_HW_PARAM;
    }

    CFFmpegDecoder* decoder = (CFFmpegDecoder*)handle;
    int ret = decoder->startWork(dataCallBack, pUserData);
    MLOG_WARN("Deocde Over!!");
    return ret != 0 ? ERR_HW_FFMPEG : ERR_HW_SUCCESS;
}

/**
 * @fn	bstar_decoder_stop_decode
 * @brief 停止解复用.
 * @param[in] handle 解复用实例
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
int bstar_decoder_stop_decode(DECODER_HANDLE handle)
{
    if (nullptr == handle)
    {
        MLOG_ERROR("Invalid Input Param!!");
        return ERR_HW_PARAM;
    }

    CFFmpegDecoder* decoder = (CFFmpegDecoder*)handle;
    int ret = decoder->stopWork();
    if (ret != 0)
    {
        MLOG_ERROR("Stop Decoder Err!!url[%s]", decoder->getVideoUrl().c_str());
    }
    
    return ret == 0 ? ERR_HW_SUCCESS : ERR_HW_FFMPEG;
}

/**
 * @fn	bstar_decoder_del_instance
 * @brief 删除解复用实例.
 * @param[in] handle 解复用实例
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
int bstar_decoder_del_instance(DECODER_HANDLE handle)
{
    if (nullptr == handle)
    {
        MLOG_ERROR("Invalid Input Param!!");
        return ERR_HW_PARAM;
    }

    CFFmpegDecoder* decoder = (CFFmpegDecoder*)handle;
    delete decoder;
    return ERR_HW_SUCCESS;
}

/**
 * @fn	bstar_hwdec_uninit
 * @brief 硬解库去初始化.
 * @param[in] 无
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
int bstar_hwdec_uninit()
{
    avformat_network_deinit();
    return 0;
}

// 获取错误信息
const char* bstar_hwdec_get_errmsg(int errno)
{
    auto iter = g_errMap.find((E_HwErr)errno);
    if (iter != g_errMap.end())
    {
        return iter->second.c_str();
    }
    else
    {
        return g_errMap[ERR_HW_UNKNOW].c_str();
    }
}