#!/bin/bash

#
# 编译库
#
build_dir="build_linux"
rm -rf ${build_dir} 
mkdir ${build_dir}
cd ${build_dir}
cmake ../ -DCMAKE_BUILD_TYPE=debug
cmake --build .
cd ..

echo ""
echo ""
echo "Build To Dir:build_linux"
echo ""
echo ""
#
# 打包库提供第三方使用
#
out_dir="packet_linux"
if [ "$1" == "pack" ];
then
    rm -rf ${out_dir}

    echo "Begin Packet Out!!"
    mkdir -p ${out_dir}/include ${out_dir}/lib
    # 头文件
    cp ./include/*.h ${out_dir}/include
    cp ./common/common.h ${out_dir}/include
    # 库
    cp ${build_dir}/libHwDec* ${out_dir}/lib
    # ffmpeg 库
    cp -r ./thirdLib/ffmpeg-5.1.2/linux/lib/* ${out_dir}/lib
    echo "Packet To Dir:${out_dir}"
fi

