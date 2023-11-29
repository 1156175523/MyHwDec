#include "FFmpegDecoder.h"
#include "utils.h"

#include <thread>

extern int DEBUG_WRITE_FILE;

CFFmpegDecoder::CFFmpegDecoder()
{
    m_inputCtx = nullptr;
#if 0
    m_codecCtxV = nullptr;
    m_codecV = nullptr;
    m_codecCtxA = nullptr;
    m_codecA = nullptr;
#endif
    m_video = nullptr;
    m_audio = nullptr;
    m_vStreamIndex = -1;
    m_aStreamIndex = -1;
    m_videoUrl = "";
    isInit = false;
    isStop = false;
    isDealDataOver = true;
    memset(&m_streamInfo, 0x00, sizeof(m_streamInfo));
}

CFFmpegDecoder::CFFmpegDecoder(std::string url):m_videoUrl(url)
{
    m_inputCtx = nullptr;
#if 0
    m_codecCtxV = nullptr;
    m_codecV = nullptr;
    m_codecCtxA = nullptr;
    m_codecA = nullptr;
#endif
    m_video = nullptr;
    m_audio = nullptr;
    m_vStreamIndex = -1;
    m_aStreamIndex = -1;
    isInit = false;
    isStop = false;
    isDealDataOver = true;
    memset(&m_streamInfo, 0x00, sizeof(m_streamInfo));
}

CFFmpegDecoder::~CFFmpegDecoder()
{
}

// 获取ffmpeg错误信息
std::string CFFmpegDecoder::getFFmpegErrMsg(int errnum)
{
    std::string retStr;
    char errMsgBuf[1024] = {0};
    av_strerror(errnum, errMsgBuf, sizeof(errMsgBuf)-1);
    retStr = errMsgBuf;
    return retStr;
}

/**
 * @fn	doInit
 * @brief 初始化相关参数.
 * @param[in] 无
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
int CFFmpegDecoder::doInit()
{
    int ret = 0;
    AVDictionary *format_opts = NULL;

    ret = av_dict_set(&format_opts, "rtsp_transport", "tcp", 0);                         // 设置取流的方式，默认udp。- 目前测试h265不使用tcp方式会丢包导致花屏
    if (ret != 0) {
        MLOG_WARN("AV_DICT_SET 'rtsp_transport' 'tcp' Err!!, ret[%d]-[%s]", ret, getFFmpegErrMsg(ret).c_str());
    }
    ret = av_dict_set(&format_opts, "stimeout", "5000000", 0);                            // 设置超时5s - 设置rtsp拉流超时时间
    if (ret != 0) {
        MLOG_WARN("AV_DICT_SET 'timeout' '5000000' Err!!, ret[%d]-[%s]", ret, getFFmpegErrMsg(ret).c_str());
    }
#ifdef _WIN32
    ret = av_dict_set(&format_opts, "timeout", "5000000", 0);                            // 设置超时5s - 设置udp,http超时
#endif
    //ret = av_dict_set(&opts, "rw_timeout", "10000000", 0);                               // rw_timeout 网络读写操作最大等待时间，微妙

    ret = avformat_open_input(&m_inputCtx, m_videoUrl.c_str(), NULL, &format_opts);
    if (ret != 0)
    {
        MLOG_ERROR("Cannot open input file '%s', ret[%d]-[%s]", m_videoUrl.c_str(), ret, getFFmpegErrMsg(ret).c_str());
        ret = -1;
        av_dict_free(&format_opts);
        goto fail;
    }
    av_dict_free(&format_opts);

    ret = avformat_find_stream_info(m_inputCtx, NULL);
    if (ret < 0)
    {
        MLOG_ERROR("Cannot find input stream information. ret[%d][%s]", ret, getFFmpegErrMsg(ret).c_str());
        ret = -1;
        goto fail;
    }

    for(unsigned int i=0; i<m_inputCtx->nb_streams; ++i)
    {
        if (m_inputCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            // 保存视频信息
            m_streamInfo.streamSum |= DATA_OF_VIDEO;
            m_streamInfo.videoInfo.videoType = (int)m_inputCtx->streams[i]->codecpar->codec_id;
            m_streamInfo.videoInfo.pixFmt = (int)m_inputCtx->streams[i]->codecpar->format;
            m_streamInfo.videoInfo.width = m_inputCtx->streams[i]->codecpar->width;
            m_streamInfo.videoInfo.height = m_inputCtx->streams[i]->codecpar->height;
            // 保存视频索引和流信息
            m_vStreamIndex = i;
            m_video = m_inputCtx->streams[i];
        }
        if (m_inputCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            // 保存音频信息
            m_streamInfo.streamSum |= DATA_OF_AUDIO;
            m_streamInfo.audioInfo.audioType = (int)m_inputCtx->streams[i]->codecpar->codec_id;
            m_streamInfo.audioInfo.bitRate = (int)m_inputCtx->streams[i]->codecpar->bit_rate;
            //m_streamInfo.audioInfo.channels = m_inputCtx->streams[i]->codecpar->ch_layout.nb_channels;    // 较新版本使用
            //m_streamInfo.audioInfo.channels = m_inputCtx->streams[i]->codecpar->channels;                 // 旧版本使用字段
            m_streamInfo.audioInfo.sampleRate = m_inputCtx->streams[i]->codecpar->sample_rate;
            // 保存音频索引和流信息
            m_aStreamIndex = i;
            m_audio = m_inputCtx->streams[i];
        }
    }

#if 0
    // 初始化视频解码器
    if (m_vStreamIndex != -1)
    {
        ret = av_find_best_stream(m_inputCtx, AVMEDIA_TYPE_VIDEO, -1, -1, (const AVCodec**)&m_codecV, 0);
        if (ret == AVERROR_DECODER_NOT_FOUND || ret == AVERROR_STREAM_NOT_FOUND)
        {
            MLOG_WARN("Video Decoder Not Found, ret[%d]-[%s]", ret, getFFmpegErrMsg(ret).c_str());
        }
        else if (ret < 0)
        {
            MLOG_ERROR("Video Decoder Found Err, ret[%d]-[%s]", ret, getFFmpegErrMsg(ret).c_str());
            ret = -1;
            goto fail;
        }

        m_codecCtxV = avcodec_alloc_context3(m_codecV);
        if (m_codecCtxV == nullptr)
        {
            MLOG_ERROR("Get Video CodecCtx Err!!");
            ret = -1;
            goto fail;
        }

        ret = avcodec_parameters_to_context(m_codecCtxV, m_video->codecpar);
        if (ret < 0)
        {
            MLOG_ERROR("Sync Video CodecParam Err! ret[%d]-[%s]", ret, getFFmpegErrMsg(ret).c_str());
            ret = -1;
            goto fail;
        }

        ret = avcodec_open2(m_codecCtxV, m_codecV, NULL);
        if (ret != 0)
        {
            MLOG_ERROR("Open Video Codec Err! ret[%d]-[%s]", ret, getFFmpegErrMsg(ret).c_str());
            ret = -1;
            goto fail;
        }
    }

    // 初始化音频解码器
    if (m_aStreamIndex != -1)
    {
        ret = av_find_best_stream(m_inputCtx, AVMEDIA_TYPE_AUDIO, -1, -1, (const AVCodec**)&m_codecA, 0);
        if (ret == AVERROR_DECODER_NOT_FOUND || ret == AVERROR_STREAM_NOT_FOUND)
        {
            MLOG_WARN("Audio Decoder Not Found, ret[%d]-[%s]", ret, getFFmpegErrMsg(ret).c_str());
        }
        else if (ret < 0)
        {
            MLOG_ERROR("Audio Decoder Found Err, ret[%d]-[%s]", ret, getFFmpegErrMsg(ret).c_str());
            goto fail;
        }

        m_codecCtxA = avcodec_alloc_context3(m_codecA);
        if (m_codecCtxA == nullptr)
        {
            MLOG_ERROR("Get Audio CodecCtx Err!!");
            ret = -1;
            goto fail;
        }

        ret = avcodec_parameters_to_context(m_codecCtxA, m_audio->codecpar);
        if (ret < 0)
        {
            MLOG_ERROR("Sync Audio CodecParam Err! ret[%d]-[%s]", ret, getFFmpegErrMsg(ret).c_str());
            ret = -1;
            goto fail;
        }
        
        ret = avcodec_open2(m_codecCtxA, m_codecA, NULL);
        if (ret != 0)
        {
            MLOG_ERROR("Open Audio Codec Err! ret[%d]-[%s]", ret, getFFmpegErrMsg(ret).c_str());
            ret = -1;
            goto fail;
        }
    }
#endif

/*
    if (report){
        delete report;
        report = nullptr;
    }
*/
    isInit = true;
    return ret >= 0 ? 0 : -1;

fail:
/*
    if (report){
        delete report;
        report = nullptr;
    }
*/
    unInit();
    return ret;
}

/**
 * @fn	unInit
 * @brief 去初始化.
 * @param[in] 无
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
int CFFmpegDecoder::unInit()
{
#if 0
    if (m_codecCtxA){
        avcodec_free_context(&m_codecCtxA);
    }
    if (m_codecCtxV){
        avcodec_free_context(&m_codecCtxV);
    }
#endif
    if (m_inputCtx){
        avformat_close_input(&m_inputCtx);
    }
    m_video = nullptr;
    m_audio = nullptr;
    m_aStreamIndex = m_vStreamIndex = -1;
    isInit = false;

    return 0;
}

/**
 * @fn	getStreamInfo
 * @brief 获取视频信息.
 * @param[in] streamInfo 输入流信息
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
int CFFmpegDecoder::getStreamInfo(StreamInfo* streamInfo)
{
    int ret = 0;
    if (!streamInfo)
    {
        MLOG_ERROR("Invalid Input Param!");
        return -1;
    }

    if (!isInit)
    {
        ret = doInit();
        if (ret == 0){
            MLOG_INFO("Decoder Do Init Success!!");
            isInit = true;
        }else{
            MLOG_ERROR("Decoder Do Init Error!!");
        }
    }

    if (ret == 0)
    {
        memcpy(streamInfo , &m_streamInfo, sizeof(m_streamInfo));
    }

    return ret;
}

/**
 * @fn	startWork
 * @brief 开始解复用数据.
 * @param[in] dataCallback 数据回调函数
 * @param[in] pUserData 回调函数用户自定义入参
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
int CFFmpegDecoder::startWork(VideoCallBack dataCallback, void* pUserData)
{
    int ret = 0;
    if (!isInit)
    {
        if (!doInit())
        {
            return -1;
        }
    }

    AVPacket *packet = nullptr;
    packet = av_packet_alloc();
    if (packet == nullptr)
    {
        MLOG_ERROR("Malloc Packet Err!!");
        return -1;
    }

    isDealDataOver = false;
    while(!isStop)
    {
        ret = av_read_frame(m_inputCtx, packet);
        if (ret < 0)
        {
            MLOG_ERROR("Read Frame Err!! ret[%d]-[%s]", ret, getFFmpegErrMsg(ret).c_str());
            break;
        }

        StreamData* pStreamData = nullptr;
        pStreamData = (StreamData*)malloc(sizeof(StreamData));
        memset(pStreamData, 0x00, sizeof(StreamData));
        pStreamData->dataLen = packet->size;
        pStreamData->data = malloc(packet->size);
        memset(pStreamData->data, 0x0, packet->size);
        memcpy(pStreamData->data, packet->data, packet->size);

        if (m_vStreamIndex == packet->stream_index)
        {
            // 视频数据
            pStreamData->dataType = DATA_OF_VIDEO;
            if (DEBUG_WRITE_FILE)
            {
                // 写视频数据
                char fileName[256] = {0};
                snprintf(fileName, sizeof(fileName) - 1, "video_codecId%d_%p.data", m_video->codecpar->codec_id, this);
                FILE *file = fopen((const char *)fileName, "ab+");
                fwrite(packet->data, 1, packet->size, file);
                fclose(file);
            }
        }

        if (m_aStreamIndex == packet->stream_index)
        {
            // 音频数据
            pStreamData->dataType = DATA_OF_AUDIO;
            if (DEBUG_WRITE_FILE)
            {
                // 写音频数据
                char fileName[256] = {0};
                snprintf(fileName, sizeof(fileName) - 1, "audio_codecId%d_%p.data", m_video->codecpar->codec_id, this);
                FILE *file = fopen((const char *)fileName, "ab+");
                fwrite(packet->data, 1, packet->size, file);
                fclose(file);
            }
        }

        // 执行用户回调 - 目前只处理音频和视频流
        if (packet->stream_index == m_aStreamIndex || packet->stream_index == m_vStreamIndex)
        {
            if (dataCallback)
            {
                // 用户回调结束后数据失效
                dataCallback(pStreamData, pUserData);
            }
        }

#if DEBUG_WRITE_FILE
            // 写音视频数据
            char fileName[256]={0};
            std::string vStr = m_video ? "V_" + std::to_string(int(m_video->codecpar->codec_id)) + "_" : "";
            std::string aStr = m_audio ? "A_" + std::to_string(int(m_audio->codecpar->codec_id)) + "_" : "";
            
            snprintf(fileName, sizeof(fileName)-1, "stream_%s_%s_%p.data", \
                                    vStr.c_str(), aStr.c_str(), this);
            FILE* file = fopen((const char*)fileName, "ab+");
            fwrite(packet->data, 1, packet->size, file);
            fclose(file);
#endif

        av_packet_unref(packet);
    }

    av_packet_free(&packet);
    isDealDataOver = true;
    return ret != 0 ? -1 : 0;
}

/**
 * @fn	stopWork
 * @brief 停止解复用数据.
 * @param[in] 无
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
int CFFmpegDecoder::stopWork()
{
    isStop = true;
    while(!isDealDataOver){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    unInit();
    return 0;
}