# MyHwDec
 use ffmpeg for hardware decode
# 目录介绍
├── build_linux.sh

├── build_win.bat

├── CMakeLists.txt

├── common

│   ├── common.h

│   └── utils.h

├── config

│   └── config.ini               // 配置文件

├── include

│   └── hw_decoder_api.h         // 对外c接口

├── sample                       // 测试案例

│   ├── build_linux.sh

│   ├── build_win.bat

│   ├── CMakeLists_test.txt

│   ├── CMakeLists.txt

│   ├── CMakeLists.txt.backup

│   └── test_api.cpp

├── src

│   ├── hw_decoder_api.cpp

│   ├── utils.cpp

│   └── videoOption              // 封装解复用类和硬解类(解耦)

│       ├── FFmpegDecoder.cpp

│       ├── FFmpegDecoder.h

│       ├── HwDecoder.cpp

│       └── HwDecoder.h

├── thirdLib                     // 第三方依赖，目前是ffmpeg的依赖库，用户可以使用自己的依赖不依赖此路径

    ├── ffmpeg-5.1.2
    
    └── libevent-2.1.12          // 未使用到
    

1、目前该项目只在64位环境测试通过

2、文件介绍

   ①build_linux.sh、build_win.bat 分别是linux和windows下编译脚本,使用者可以在已安装ffmpeg的环境下提取
    common、include、src三个目录进行自定义编译测试

   ②config/config.ini 主要是一些配置目前包括设置ffmpeg日志级别、是否输出debug文件(解复用的数据、解码后的数据)、指定硬解类型(优先级低于接口设置的类型)
   
3、项目头文件中可能有一些为用到的结构用户可以自定义剪裁

4、核心代码在src/videoOption

   - FFmpegDecoder.*只要是获取输入url的信息和解复用数据(编码后的音视频数据)
     
   - HwDecoder.* 支持自动查找目前系统中支持的硬解类型并应用到解码器,支持逐帧解码视频数据(目前只支持解码视频数据)
