# 检查硬解库是否编译
CUR_DIR=`pwd`
DEPEND_DIR=${CUR_DIR}/../build_linux
if [ ! -d "${DEPEND_DIR}" ]; then
    echo "Depend Lib Need Complie First!!"
    exit 1
fi

rm -rf build_linux
mkdir build_linux
cd build_linux
cmake .. -DCMAKE_BUILD_TYPE=debug
cmake --build .
cd ..

echo "Build To Dir:build_linux"