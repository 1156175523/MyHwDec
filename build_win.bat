@echo off

:: Windows 环境得安装 MinGW 环境

::
:: 进行库编译 - 目前边编译类型为debug版本
::
set "compile_dir=%CD%\build_win"
IF EXIST "%compile_dir%" (
    echo The Compile Directory Exist, Current Begin Delete.
    rmdir /s /q %compile_dir%
    IF %errorlevel% == 1 (
        goto fail_complie
    )
) ELSE (
    echo The Complie Directory Does Not Exist, Will Create One.
)

mkdir build_win
IF %errorlevel% == 1 (
    goto fail_complie
)

cd build_win
IF %errorlevel% == 1 (
    goto fail_complie
)

cmake .. -DCMAKE_BUILD_TYPE=debug -G "MinGW Makefiles"
IF %errorlevel% == 1 (
    cd ..
    goto fail_complie
)

cmake --build .
IF %errorlevel% == 1 (
    cd ..
    goto fail_complie
)

cd ..
IF %errorlevel% == 1 (
    goto fail_complie
)

echo.
echo.
echo "Complie Success, Traget To Dir:build_win"
echo.
echo.

::
:: 开始将编译的内容打包到指定目录,提供第三方使用
::
if "%1" == "pack" (

echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
echo "Now Begin Packet..."
set out_dir=%CD%\packet_win
IF EXIST "%out_dir%" (
    echo The Out Directory Exist, Current Begin Delete.
    rmdir /s /q %out_dir%
    IF %errorlevel% == 1 (
        goto fail_build
    )
) ELSE (
    echo The Out Directory Does Not Exist, Will Create One.
)

mkdir %out_dir%
if %errorlevel% == 1 (
    goto fail_build
)

set fileExtension=dll

:: 拷贝编译后的库
xcopy "%compile_dir%\*dll" "%out_dir%\lib"  /C /I
if %errorlevel% == 1 (
    goto fail_build
)
:: ffmpeg运行库
xcopy "%CD%\thirdLib\ffmpeg-5.1.2\win\bin\*" "%out_dir%\lib"  /C /I
if %errorlevel% == 1 (
    goto fail_build
)

:: 拷贝头文件
xcopy "%CD%\include\*.h" "%out_dir%\include" /C /I
if %errorlevel% == 1 (
    goto fail_build
)
xcopy "%CD%\common\common.h" "%out_dir%\include" /C /I
if %errorlevel% == 1 (
    goto fail_build
)

echo "Packet Success, Target to:%out_dir%"
exit /b


) ELSE (
    exit /b
)

:: 出错
:fail_complie
echo "Fail To Complie Err!!"
exit /b

:fail_build
echo "Fail To Build Err!!"
exit /b
