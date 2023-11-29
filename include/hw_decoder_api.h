#ifndef __HW_DECODER_H___
#define __HW_DECODER_H___

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
//#define MY_LIB_API extern "C" __declspec(dlliexport)
//#define DMEC __declspec(dllexport)
#define MY_LIB_API __declspec(dllexport)
#else
#define MY_LIB_API
#endif


#include "common.h"

/**
 * @fn	bstar_hwdec_doinit
 * @brief 硬解库初始化.
 * @param[in] 无
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
MY_LIB_API int bstar_hwdec_doinit(const char* configFile);

/***************
 * 硬解相关接口
****************/
/**
 * @fn	bstar_hwdec_get_instance
 * @brief 获取解码实例.
 * @param[in] 无
 * @param[out] 无
 * @return
 * @retval NULL-成功 !NULL-失败
 */
MY_LIB_API HW_HADNLE bstar_hwdec_get_instance();
/**
 * @fn	bstar_hwdec_get_instance
 * @brief 设置硬解类型.
 * @param[in] handle  硬解实例
 * @param[in] hwType  硬解类型,d3d11va,qsv,vaapi等,如果支持则使用设置的硬解类型,如果不支持则进行软解
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
MY_LIB_API int bstar_hwdec_set_type(HW_HADNLE handle, const char* hwType);
/**
 * @fn	bstar_hwdec_set_input_desc
 * @brief 设置需要进行硬解的原始数据信息.
 * @param[in] handle  硬解实例
 * @param[in] inputDesc 输入信息描述
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
MY_LIB_API int bstar_hwdec_set_input_desc(HW_HADNLE handle, StrmInputDesc* inputDesc);
/**
 * @fn	bstar_hwdec_del_instance
 * @brief 删除解码实例.
 * @param[in] handle
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
MY_LIB_API int bstar_hwdec_del_instance(HW_HADNLE handle);
/**
 * @fn	bstar_hwdec_push_data
 * @brief 推送需要解码的视频数据.
 * @param[in] handle 硬解实例
 * @param[in] srcData 输入是视频编码数据(h264,h265等数据包)
 * @param[out] result 解码后的原始视频数据
 * @return
 * @retval 0-成功 !0-失败
 */
MY_LIB_API int bstar_hwdec_push_data(HW_HADNLE handle, SrcData *srcData, DecodeData* result);

/*********************************
 * 解复用相关接口-获取编码后的数据
**********************************/
// 解复用数据回调函数
typedef int BStarVideoCallBack(StreamData* pStreamData, void* pUserData);
/**
 * @fn	bstar_decoder_get_instance
 * @brief 根据输入url获取解码实例.
 * @param[in] fileUrl 输入流信息,rtsp,rtmp,本地文件
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
MY_LIB_API DECODER_HANDLE bstar_decoder_get_instance(const char* fileUrl);
/**
 * @fn	bstar_decoder_get_stream_info
 * @brief 获取输入url的流信息.
 * @param[in] handle 解复用实例
 * @param[out] pStreamInfo 流信息
 * @return
 * @retval 0-成功 !0-失败
 */
MY_LIB_API int bstar_decoder_get_stream_info(DECODER_HANDLE handle, StreamInfo* pStreamInfo);
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
MY_LIB_API int bstar_decoder_start_decode(DECODER_HANDLE handle, VideoCallBack dataCallBack, void* pUserData);
/**
 * @fn	bstar_decoder_stop_decode
 * @brief 停止解复用.
 * @param[in] handle 解复用实例
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
MY_LIB_API int bstar_decoder_stop_decode(DECODER_HANDLE handle);
/**
 * @fn	bstar_decoder_del_instance
 * @brief 删除解复用实例.
 * @param[in] handle 解复用实例
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
MY_LIB_API int bstar_decoder_del_instance(DECODER_HANDLE handle);

/**
 * @fn	bstar_hwdec_uninit
 * @brief 硬解库去初始化.
 * @param[in] 无
 * @param[out] 无
 * @return
 * @retval 0-成功 !0-失败
 */
MY_LIB_API int bstar_hwdec_uninit();
// 获取错误信息
MY_LIB_API const char* bstar_hwdec_get_errmsg(int errno);

#ifdef __cplusplus
}
#endif

#endif