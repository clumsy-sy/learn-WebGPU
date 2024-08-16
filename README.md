# learn-WebGPU

## 编译

## Linux

```sh
# 下载依赖 & 编译本体
make all

# 编译到 wasm
git clone https://github.com/emscripten-core/emsdk.git /
cd emsdk /
./emsdk install latest /
./emsdk activate latest /
cd ..
make emscripten

# 运行
make run
```

## windows

```sh
# C++ 编译
.\winBuild.bat

# 编译到 wasm
.\winBuild.bat emscripten

# 运行
.\winBuild.bat run

```

### 编译问题


针对 `libprotobuf-mutator` 编译不过的问题
对 `build/_deps/dawn-src/third_party/CMakeLists.txt` 中的 `libprotobuf-mutator` 部分进行注释

```cmake
# if (NOT TARGET libprotobuf-mutator)
#     message(STATUS "Dawn: using LPM at ${DAWN_LPM_DIR}")
#     include("libprotobuf-mutator/BUILD.cmake")
# endif()
```