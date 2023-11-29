#include "HwDecoder.h"
#include "utils.h"

extern int DEBUG_WRITE_FILE;
extern std::string g_userCustomHwDevice;
////////////////////////////////////////////////
// 获取像素格式
static enum AVPixelFormat GetPixFmt(struct AVCodecContext* s, const enum AVPixelFormat* pix_fmts)
{
    if (pix_fmts == NULL)
        return AV_PIX_FMT_NONE;

    AVPixelFormat cmpPixFmt = ((CHWDecoder*)s->opaque)->m_hwPixFmt;
    std::string hadPix="";

    const enum AVPixelFormat *p;
    for (p = pix_fmts; *p != AV_PIX_FMT_NONE; p++) {
        //MLOG_WARN(">>> PIX_FMT[%d]", (int)*p);
        hadPix += std::to_string(int(*p)) + "|";
        if (*p == cmpPixFmt)
            return *p;
    }

    MLOG_ERROR("Failed to get HW surface format!!");
    MLOG_ERROR("Haved:[%s] -- need[%d]", hadPix.c_str(), int(cmpPixFmt));
    //return AV_PIX_FMT_NONE;
    return s->sw_pix_fmt;   // 当检查支持硬解但是实际解码不支持时会返回这个值,保证可以进行正常解码(目前推测这里进行了软解)
}

CHWDecoder::CHWDecoder()
{
    m_isHwDec = true;   // 默认硬解
    m_codecCtx = nullptr;
    m_codec = nullptr;
    m_hwDeviceCtx = nullptr;
    m_hwDecType = AV_HWDEVICE_TYPE_NONE;
    m_hwPixFmt = AV_PIX_FMT_NONE;
    m_hwDecType = "";
}

CHWDecoder::~CHWDecoder()
{
}

// 获取ffmpeg错误信息
std::string CHWDecoder::getFFmpegErrMsg(int errnum)
{
    std::string retStr;
    char errMsgBuf[1024] = {0};
    av_strerror(errnum, errMsgBuf, sizeof(errMsgBuf)-1);
    retStr = errMsgBuf;
    return retStr;
}

/**
 * @fn	getLocalSupported
 * @brief 获取本地支持硬解类型.
 * @param[in] 无
 * @param[out] std::vector<std::string>& supportHwVecOut [d3dva,cuda,vaapi...]
 * @return
 * @retval =0: 成功 <0: 失败
 */
int CHWDecoder::getLocalSupported(std::vector<std::string>& supportHwVecOut)
{
	AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
	while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE) {
		//get_hw
		supportHwVecOut.push_back(av_hwdevice_get_type_name(type));
		MLOG_INFO("- %s", av_hwdevice_get_type_name(type));
	}
	return 0;
}

/**
 * @fn	hwDecodeInit
 * @brief 初始化硬件上下文.
 * @param[in] 无
 * @param[out] 无
 * @return
 * @retval =0: 成功 <0: 失败
 */
int CHWDecoder::hwDecodeInit()
{
    int ret = 0;

    if ((ret = av_hwdevice_ctx_create(&m_hwDeviceCtx, m_hwDevType, NULL, NULL, 0)) < 0)
    {
        MLOG_ERROR("Failed to create specified HW device. ret[%d] errMsg[%s]", ret, getFFmpegErrMsg(ret).c_str());
        return ret;
    }

    return ret;
}

/**
 * @fn	CodecInit
 * @brief 根据输入视频数据获取硬解码器.
 * @param[in] 无
 * @param[out] 无
 * @return
 * @retval =1: 找到合适的硬解设备 =0: 未找到合适的硬解设备 <0: 出错
 */
int CHWDecoder::codecInit()
{
    m_codec = (AVCodec*)avcodec_find_decoder(m_inputCodecId);
    if (!m_codec)
    {
        MLOG_ERROR("codecId[%d] Can't Find Decoder!!", (int)m_inputCodecId);
        return -1;
    }
    m_codecCtx = avcodec_alloc_context3(m_codec);
    if (!m_codecCtx)
    {
        MLOG_ERROR("Malloc codecCtx Err!!");
        return -1;
    }

    return 0;
}

/**
 * @fn	checkHwDeviceSupport
 * @brief 指定硬解类型检查.
 * @param[in] hwTypeStr eg:cuda,d3d11va,vaapi...
 * @param[out] 无
 * @return
 * @retval =1: 找到合适的硬解设备 =0: 未找到合适的硬解设备 <0: 出错
 */
bool CHWDecoder::checkHwDeviceSupport(std::string hwTypeStr)
{
    bool bRet = false;
    int iRet = 0;

    AVHWDeviceType type = av_hwdevice_find_type_by_name(hwTypeStr.c_str());
    if (type == AV_HWDEVICE_TYPE_NONE)
    {
        MLOG_ERROR("hwTypeStr[%s] Not Found!!", hwTypeStr.c_str());
        return false;
    }

    // 查看输入数据类型编码器的硬解配置
    for (int i = 0;; i++)
    {
        const AVCodecHWConfig *config = avcodec_get_hw_config(m_codec, i);
        if (!config)
        {
            MLOG_WARN("Decoder [%s] does not support device type [%s].\n",
                      m_codec->name, av_hwdevice_get_type_name(type));
            break;
        }

        // 找到合适的硬解码器
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == type)
        {
            if ((iRet = av_hwdevice_ctx_create(&m_hwDeviceCtx, type, NULL, NULL, 0)) < 0)
            {
                MLOG_ERROR("Failed to create specified HW device. hwTypeStr[%s] ret[%d] errMsg[%s]", 
                                       hwTypeStr.c_str() , iRet, getFFmpegErrMsg(iRet).c_str());
                continue;
            }
            else
            {
                // 支持硬解
                bRet = true;

                // 保存硬解相关参数
                m_hwDevType = type;
                m_hwPixFmt = config->pix_fmt;
                break;
            }
        }
    }

    return bRet;
}

/**
 * @fn	findSuitableHwDecoder
 * @brief 根据输入视频数据获取硬解码器.
 * @param[in] 无
 * @param[out] 无
 * @return
 * @retval =1: 找到合适的硬解设备 =0: 未找到合适的硬解设备 <0: 出错
 */
int CHWDecoder::findSuitableHwDecoder()
{
    int ret = 0;
    m_isHwDec = false;  // 获取合适的硬解码器时,初始化为false

    // 获取当前机器支持的所有硬解
    getLocalSupported(m_supportHwVec);
    if (m_supportHwVec.size() == 0)
    {
        MLOG_INFO("supportHwVecOut size is 0, not support hwdec!!");
        return 0;
    }
    // 初始化解码器和解码器上下文
    ret = codecInit();
    if (ret != 0)
    {
        MLOG_ERROR("codecInit Err!!");
        return -1;
    }

    // 自动选择
    if (m_hwDecType.empty() && g_userCustomHwDevice.empty())
    {
        bool isFind = false;    // 是否找到合适的

        for (auto &hwDecTypeVar : m_supportHwVec)
        {
            bool bret = checkHwDeviceSupport(hwDecTypeVar);
            if (bret)
            {
                // 找到第一个可以使用的硬解设备
                isFind = true;
                m_hwDecType = hwDecTypeVar;
                break;
            }
        }

        // 如果找到支持的硬解设备
        if (isFind)
        {
            m_isHwDec = true;
            m_codecCtx->get_format = GetPixFmt;
            m_codecCtx->opaque = this;          // 检查像素格式使用
            m_codecCtx->hw_device_ctx = av_buffer_ref(m_hwDeviceCtx);   // 绑定硬解上下文
        }

    }
    // 指定硬解类型
    else
    {
        // - 根据指定硬解查找
        if (m_hwDecType.empty())
        {
            m_hwDecType = g_userCustomHwDevice;
        }
        bool bret = checkHwDeviceSupport(m_hwDecType);
        if (bret)
        {
            m_isHwDec = true;
            m_codecCtx->get_format = GetPixFmt;
            m_codecCtx->opaque = this;          // 检查像素格式使用
            m_codecCtx->hw_device_ctx = av_buffer_ref(m_hwDeviceCtx);   // 绑定硬解上下文
        }
    }

    if (m_isHwDec)
    {
        MLOG_INFO("Stream Type[%s] Begin HardDec [%s]-[%d] hw_pix_fmt[%d].", m_codec->name, m_hwDecType.c_str(), (int)m_hwDevType, (int)m_hwPixFmt);
    }
    else
    {
        MLOG_INFO("Stream Type[%s] Begin SoftDec.", m_codec->name);
    }

    // 设置其他参数
    m_codecCtx->thread_count = 4;              // 设置解码器使用的线程数量
    m_codecCtx->thread_type = FF_THREAD_FRAME; // 允许帧级别的并行解码
    //m_codecCtx->thread_slice_count = 4;        // 设置并行解码的片段数

    // 打开解码器
    ret = avcodec_open2(m_codecCtx, m_codec, NULL);
    if (ret < 0)
    {
        MLOG_ERROR("avcodec_open2 err!! ret[%d] errMsg[%s]", ret, getFFmpegErrMsg(ret).c_str());
        return -1;
    }

    return ret;
}

/**
 * @fn	doDecode
 * @brief 进行解码.
 * @param[in] pData 需要解析的数据
 * @param[in] dataLen 需要解析的数据长度
 * @param[out] result 解码后数据
 * @return
 * @retval =0: 成功 =1: 放入数据重试 2: 取流结尾 -1:失败
 */
int CHWDecoder::doDecode(void *pData, unsigned int dataLen, DecodeData *result)
{
    if (!pData || !dataLen || !result)
    {
        MLOG_ERROR("Invalid Prm pData[%p] dataLen[%u]", pData, dataLen);
        return -1;
    }

    result->size = 0;
    result->data = nullptr;
    result->pixFmt = 0;

    int ret = 0;
    AVFrame* frame = NULL, * sw_frame = NULL;
    AVFrame* tmp_frame = NULL;
    uint8_t* buffer = NULL;
    AVPacket* packet = NULL;
    int size = 0;

    do{

        packet = av_packet_alloc();
        if (!packet)
        {
            ret = -1;
            MLOG_ERROR("Malloc Packet Err!!");
            break;
        }

        packet->data = (uint8_t *)pData;
        packet->size = (int)dataLen;

        // 发送解码器数据
        ret = avcodec_send_packet(m_codecCtx, packet);
        if (ret != 0)
        {
            MLOG_ERROR("Error during decoding packet[%p] size[%d], ret[%d] errMsg[%s]",\
                        packet->data, packet->size, ret, getFFmpegErrMsg(ret).c_str());
            ret = -1;
            break;
        }

        // 初始化数据输出数据
        if (!(frame = av_frame_alloc()) || !(sw_frame = av_frame_alloc()))
        {
            MLOG_ERROR("Can not alloc frame");
            //ret = AVERROR(ENOMEM);
            ret = -1;
            break;
        }

        // 获取解码数据
        ret = avcodec_receive_frame(m_codecCtx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            if (ret == AVERROR(EAGAIN)) {
                MLOG_WARN("Recive Frame Again!!");
                ret = 1;
            }else{
                MLOG_WARN("Recive Frame EOF!!");
                ret = 2;
            }
        
            break;
        }
        else if (ret < 0)
        {
            MLOG_ERROR("Error while decoding, ret[%d]-[%s]", ret, getFFmpegErrMsg(ret).c_str());
            ret = -1;
            break;
        }

        // 如果是硬解获取主机内存数据
        if (m_isHwDec)
        {
            if (frame->format == m_hwPixFmt)
            {
                //MLOG_WARN("frame->format[%d] m_hwPixFmt[%d] dstFmt[%d]", frame->format, m_hwPixFmt, sw_frame->format);
                /* retrieve data from GPU to CPU */
                ret = av_hwframe_transfer_data(sw_frame, frame, 0);
                if (ret < 0)
                {
                    MLOG_ERROR("Error transferring the data to system memory, ret[%d] errMsg[%s]",
                               ret, getFFmpegErrMsg(ret).c_str());
                    ret = -1;
                    break;
                }
                tmp_frame = sw_frame;
            }
            else
            {
                tmp_frame = frame;
            }
        }
        else
        // 软解
        {
            tmp_frame = frame;
        }

        // 获取数据
        size = av_image_get_buffer_size((AVPixelFormat)(tmp_frame->format), tmp_frame->width, tmp_frame->height, 1);
        //MLOG_INFO(">>>>size[%06d] pix_format[%d]\n", size, (int)tmp_frame->format);
        buffer = (uint8_t*)av_malloc(size);
        if (!buffer)
        {
            MLOG_ERROR("Can not alloc buffer");
            //ret = AVERROR(ENOMEM);
            ret = -1;
            break;
        }
        ret = av_image_copy_to_buffer(buffer, size,\
                (const uint8_t* const*)tmp_frame->data,\
                (const int*)tmp_frame->linesize, (AVPixelFormat)(tmp_frame->format),\
                tmp_frame->width, tmp_frame->height, 1);
        if (ret < 0)
        {
            MLOG_ERROR("Can not copy image to buffer,ret[%d] errMsg[%s]", ret, getFFmpegErrMsg(ret).c_str());
            ret = -1;
            break;
        }

        // 输出
        result->pixFmt = (int)tmp_frame->format;
        result->size = size;
        result->width = tmp_frame->width;
        result->height = tmp_frame->height;

#if 0
        result->data = buffer;
#else
        result->data = malloc(size);
        if (!result->data) 
        {
            MLOG_ERROR("Malloc Data Mem Err!!");
            ret = -1;
            break;
        }
        memcpy(result->data, buffer, size);
#endif

        if (DEBUG_WRITE_FILE)
        {
            // 原始数据写文件
            char outFileName[256] = {0};
            snprintf(outFileName, sizeof(outFileName) - 1, "./out_%p.yuv", this);

            // debug
            FILE *pFile = fopen((const char *)outFileName, "ab+");
            if (pFile != nullptr)
            {
                fwrite(buffer, 1, size, pFile);
                fclose(pFile);
            }
            else
            {
                MLOG_ERROR("fopen err [%d][%s]", errno, strerror(errno));
            }
        }

    } while (0);

    av_packet_free(&packet);
    av_frame_free(&frame);
    av_frame_free(&sw_frame);
    av_freep(&buffer);

    return ret;
}

/**
 * @fn	destroyResource
 * @brief 销毁资源.
 * @param[in] 无
 * @param[out] 无
 * @return
 * @retval 无
 */
void CHWDecoder::destroyResource()
{
    if (m_codecCtx){
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = nullptr;
    }
    if (m_codec){
        m_codec = nullptr;
    }
    if (m_hwDeviceCtx){
        av_buffer_unref(&m_hwDeviceCtx);
        m_hwDeviceCtx = nullptr;
    }
}