#pragma once


#include <cstddef>
#include <cstdint>
#include <memory>
#include <iostream>
#include <utility>
#include <stdexcept>
#include <fstream>
#include <array>
#include <filesystem>

#include <webgpu/webgpu.hpp>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

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


namespace webgpu {

constexpr float PI = 3.14159265358979323846f;

// 输出空指针导致的报错
bool checkNullPointerError(void *p, const std::string& message);

}


