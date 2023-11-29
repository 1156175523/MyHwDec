#include <string.h>
#include <stdio.h>

#include "../include/hw_decoder_api.h"
#include "../common/common.h"
#include "time.h"

#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

std::queue<SrcData *> videoDataQue;
std::mutex queMtx;
std::condition_variable cv;

int GetDataCallBack(StreamData* pStreamData, void* pUserData)
{
    static time_t tm=0;
    time_t tmTmp = time(NULL);
    if (tmTmp-tm > 1){
        tm = tmTmp;
        printf("type[%d] dataLen[%d] videoQue[%d]\n",pStreamData->dataType, pStreamData->dataLen, (int)videoDataQue.size());
    }

#if 0
    if (pStreamData->dataType & DATA_OF_VIDEO)
    {
        //printf("type[%d] dataLen[%d]\n",pStreamData->dataType, pStreamData->dataLen);
        HW_HADNLE hwDecode = (HW_HADNLE)pUserData;
        DecodeData decodeData;
        memset(&decodeData, 0x00, sizeof(decodeData));
        SrcData srcData;
        srcData.data = pStreamData->data;
        srcData.size = pStreamData->dataLen;
        bstar_hwdec_push_data(hwDecode, &srcData, &decodeData);
    }
#else
    if (pStreamData->dataType & DATA_OF_VIDEO)
    {
        SrcData* pSrcData = (SrcData*)malloc(sizeof(SrcData));
        memset(pSrcData, 0x0, sizeof(SrcData));
        pSrcData->data = pStreamData->data;
        pSrcData->size = pStreamData->dataLen;
        std::unique_lock<std::mutex> lck(queMtx);
        //videoDataQue.emplace(pSrcData);
        videoDataQue.push(pSrcData);
        cv.notify_all();
        free(pStreamData);
    }
    else
    {
        free(pStreamData->data);
        free(pStreamData);
    }
#endif

    return 0;
}

int main(int argc, char* argv[])
{
    int ret = ERR_HW_UNKNOW;
    printf("Usage:Cmd [url] [confile]");
    std::string confile = "../../config/config.ini";
    if (argc >= 3){
        confile = argv[2];
    }
    bstar_hwdec_doinit(confile.c_str());

    time_t timeB;
    time_t timeE;

    timeB = time(NULL);
    std::string url = "rtsp://admin:admin123@192.168.65.40:554/main";
    if (argc >=2){
        url = argv[1];
    }
    DECODER_HANDLE decoder = bstar_decoder_get_instance(url.c_str());
    StreamInfo streamInfo;
    memset(&streamInfo, 0x0, sizeof(streamInfo));
    ret = bstar_decoder_get_stream_info(decoder, &streamInfo);
    if (ret != 0)
    {
        exit(-1);
    }

    printf("@StreamInfo: >> sum[%d]\n", streamInfo.streamSum);
    if (streamInfo.streamSum & DATA_OF_VIDEO){
        printf("\t Video: type[%d] pix_fmt[%d] w*h(%dx%d)\n", \
            streamInfo.videoInfo.videoType, streamInfo.videoInfo.pixFmt, streamInfo.videoInfo.width, streamInfo.videoInfo.height);
    }
    if (streamInfo.streamSum & DATA_OF_AUDIO){
        printf("\t Audio: type[%d] bitRate[%d] sampleRate[%d] channels[%d]\n", \
            streamInfo.audioInfo.audioType, streamInfo.audioInfo.bitRate, streamInfo.audioInfo.sampleRate, streamInfo.audioInfo.channels);
    }
    timeE = time(NULL);
    printf("@@getStreamInfo RunTime[%lu]\n", (unsigned long)(timeE-timeB));


    for (int i=0; i<1; i++){

    std::thread tdDecode([&]{
        HW_HADNLE hwHandle = bstar_hwdec_get_instance();
        StrmInputDesc inputDesc;
        memset(&inputDesc, 0x00, sizeof(inputDesc));
        inputDesc.streamType = EStreamType::STREAM_TYPE_BSTAR;
        inputDesc.overLoader.videoType.codecId = streamInfo.videoInfo.videoType;
        bstar_hwdec_set_input_desc(hwHandle, &inputDesc);
        
        while(1){
            std::unique_lock<std::mutex> lck(queMtx);
            while(videoDataQue.size() == 0){
                cv.wait(lck);
            }
            SrcData* pSrcData = videoDataQue.front();
            videoDataQue.pop();
            lck.unlock();

            DecodeData result;
            memset(&result, 0x00, sizeof(result));
            bstar_hwdec_push_data(hwHandle, pSrcData, &result);

            static time_t tm = 0;
            time_t tmTmp = time(NULL);
            if (tmTmp - tm > 1)
            {
                tm = tmTmp;
                printf("Get Result pix_fmt[%d] datalen[%d] w*h(%dx%d)\n", result.pixFmt, result.size, result.width, result.height);
            }

            free(pSrcData->data);
            free(pSrcData);
            free(result.data);
        }
    });
    tdDecode.detach();

    }

    bstar_decoder_start_decode(decoder, GetDataCallBack, NULL);


    bstar_hwdec_uninit();


    return ret;
}