// 只可以引入一次
#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "global.h"


namespace webgpu {

// 输出空指针导致的报错
bool checkNullPointerError(void *p, const std::string& message){
  if(p == nullptr){
    LOG("Null pointer error: %s", message.c_str());
    return false;
  }
  return true;
}

}