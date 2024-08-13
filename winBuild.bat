@echo off

REM 检查是否有传入参数
if "%~1"=="" (
    echo No input parameters detected. Compiling with CMake...

    REM 检查是否存在 build 文件夹，如果不存在则创建
    if not exist build (
        mkdir build
        echo Created build directory.
    )

    REM 使用 CMake 生成构建系统
    cmake -S . -B build -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=1
    @REM cmake . -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON -B build

    REM 使用 CMake 编译项目
    cmake --build build --parallel 16
    
) else if "%1"=="run" (
    REM 检查是否存在 ./build/Debug/App.exe
    if exist ".\build\Debug\App.exe" (
        REM 执行 ./build/Debug/App.exe
        .\build\Debug\App.exe
    ) else (
        echo Error: ./build/Debug/App.exe not found.
        exit /b 1
    )
) else if "%1"=="EMSCRIPTEN" (
    echo Input parameter detected: EMSCRIPTEN. Compiling with EMSCRIPTEN support...

    REM 检查是否存在 build 文件夹，如果不存在则创建
    if not exist build_emscripten (
        mkdir build_emscripten
        echo Created build_emscripten directory.
    )

    REM 使用 CMake 生成构建系统并启用 EMSCRIPTEN
    cmake -S . -B build_emscripten -DCMAKE_TOOLCHAIN_FILE=path/to/emscripten.cmake
    @REM cmake . -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON -B build_emscripten

    REM 使用 CMake 编译项目
    cmake --build build_emscripten --parallel 16
) else (
    echo Input parameters detected. Proceeding with other actions...
)