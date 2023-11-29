# CMakeList.txt: HwDec 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("HwDec")

# 显示编译细节
set(CMAKE_VERBOSE_MAKEFILE ON)

# 设置构建类型
set(CMAKE_BUILD_TYPE "Debug")

# ffmpeg 路径
set(FFMPEG_INC_PATH "")
set(FFMPEG_LIB_PATH "")
set(FFMPEG_LIBS "")

set (CMAKE_CXX_STANDARD 11)
# 设置 C/C++ 的编译选项
if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  /W3 ")
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fpermissive -Wall -fPIC")
endif()

MESSAGE(STATUS "############################################################")
MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")
 
set(mySystem "")
# linux系统
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	MESSAGE(STATUS "current platform: Linux ")
	set(mySystem "linux")
	# ffmpeg 头文件 和 库路径
	set(FFMPEG_INC_PATH 
		#/home/ap/safm/ffmpeg-4.3-x86_64/include/
		${CMAKE_CURRENT_SOURCE_DIR}/thirdLib/ffmpeg-5.1.2/${mySystem}/include
		)
	set(FFMPEG_LIB_PATH
		#/home/ap/safm/ffmpeg-4.3-x86_64/lib/
		#/home/ap/safm/ffmpeg-4.3-x86_64/lib64/
		${CMAKE_CURRENT_SOURCE_DIR}/thirdLib/ffmpeg-5.1.2/${mySystem}/lib
		)
	# ffmpeg 链接的库
	set(FFMPEG_LIBS
	# ffmpeg
	avformat
	avcodec
	avdevice
	avfilter
	avutil
	swresample
	swscale
	# ffmpeg 依赖
	postproc
	# 其他
	X11
	pthread
	lzma
	va
	va-x11
	va-drm
	z
	dl
	drm
	#mfx
	#OpenCL
	#x264
	#x265
	#fdk-aac
	vorbis
	vorbisenc
	#vpx
	)
# windows 系统
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	MESSAGE(STATUS "current platform: Windows")
	set(mySystem "win")
	# ffmpeg 头文件 和 库路径
	set(FFMPEG_INC_PATH
		${CMAKE_CURRENT_SOURCE_DIR}/thirdLib/ffmpeg-5.1.2/${mySystem}/include
		)
	set(FFMPEG_LIB_PATH
		${CMAKE_CURRENT_SOURCE_DIR}/thirdLib/ffmpeg-5.1.2/${mySystem}/lib
		)
	# ffmpeg 链接的库
	set(FFMPEG_LIBS
		# ffmpeg
		avcodec
		avdevice
		avfilter
		avformat
		avutil
		swresample
		swscale
		# 其他
		)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	MESSAGE(STATUS "current platform: FreeBSD")
ELSE ()
	MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
 
MESSAGE(STATUS "############################################################")

# ffmpeg添加宏定义
add_definitions(-D__STDC_CONSTANT_MACROS)

# 指定头文件路径
include_directories(
	# 当前项目
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/common
	${CMAKE_CURRENT_SOURCE_DIR}/src/videoOption/
	# ffmpeg
	${FFMPEG_INC_PATH}
)

#message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/thirdLib/ffmpeg-5.1.2/${mySystem}/include")

# 第三方依赖路径
link_directories(
	# ffmpeg
	${FFMPEG_LIB_PATH}
	# 当前项目
)

# 第三方依赖
set(HWDEC_DEP_LISTS
	# ffmpeg
	${FFMPEG_LIBS}
	# 其他
)

# 源文件
file(GLOB HWDEC_SRC_LISTS
    ./src/*.cpp
	./include/*.h
	# ffmpegDecoder
	./src/videoOption/*.cpp
	./src/videoOption/*.c
	./src/videoOption/*.h
	# streamIO
	#./src/streamIO/*.cpp
	#./src/streamIO/*.h
)

message(STATUS ${HWDEC_SRC_LISTS})

# 将源代码打包成库。
set(LIB_TYPE SHARED)
#set(LIB_TYPE STATIC)
add_library(${PROJECT_NAME} ${LIB_TYPE} ${HWDEC_SRC_LISTS})

# 链接第三方库
target_link_libraries(${PROJECT_NAME} ${HWDEC_DEP_LISTS})