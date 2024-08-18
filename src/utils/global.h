#pragma once


#include <cstddef>
#include <cstdint>
#include <memory>
#include <iostream>
#include <utility>
#include <stdexcept>
#include <fstream>

#include <webgpu/webgpu.hpp>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__








#define LOG_ENABLED

#ifdef LOG_ENABLED
  #define LOG(...) printf("[LOG] -- "); printf(__VA_ARGS__)
  #define LOG_M(message, ...) printf("[%s] -- ", message); printf(__VA_ARGS__)
#else
  #define LOG(...)
  #define LOG_M(...)
#endif
