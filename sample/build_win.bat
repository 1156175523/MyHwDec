@echo off
::
:: 检查依赖库是否完成编译
::
set "depend_lib_dir=%CD%\..\build_win"
IF EXIST "%depend_lib_dir%" (
    echo HwDec Library Exist!!
) ELSE (
    echo Need Complie HwDec Library First!!
    goto fail_depend
)

::
:: 编译测试案例
::

set "build_directory=%CD%\build_win"
IF EXIST "%build_directory%" (
    echo The Directory Exist, Current Begin Delete.
    rmdir /s /q %build_directory%
    IF %errorlevel% == 1 (
        goto fail
    )
) ELSE (
    echo The Directory Does Not Exist, Will Create One.
)

mkdir build_win
IF %errorlevel% == 1 (
    goto fail
)

cd build_win
IF %errorlevel% == 1 (
    goto fail
)

cmake .. -DCMAKE_BUILD_TYPE=debug -G "MinGW Makefiles"
IF %errorlevel% == 1 (
    cd ..
    goto fail
)

cmake --build .
IF %errorlevel% == 1 (
    cd ..
    goto fail
)

echo "Build Test Success!!"
echo "Begin Copy Depend Lib!!"
:: 复制硬解运行时库
xcopy %depend_lib_dir%\*.dll .\
IF %errorlevel% == 1 (
    cd ..
    goto fail_depend
)
:: 复制ffmpeg运行时库
xcopy %CD%\..\..\thirdLib\ffmpeg-5.1.2\win\bin\*.dll .\
IF %errorlevel% == 1 (
    cd ..
    goto fail_depend
)

cd ..
IF %errorlevel% == 1 (
    goto fail
)


echo "Build To Dir:build_win"
exit /b

:fail
echo "Build Err!!"
exit /b
:fail_depend
echo "Depend Err!!"
exit /b