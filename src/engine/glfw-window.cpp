#include "glfw-window.h"
#include "GLFW/glfw3.h"
#include "src/utils/global.h"

namespace webgpu {

bool WGPUGLFWWindow::initWindow(window_size_t window_size) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window = glfwCreateWindow(window_size.width, window_size.height, "WebGPU", nullptr, nullptr);
  return checkNullPointerError(window, "GLFW window");
}
WGPUGLFWWindow::WGPUGLFWWindow(){
  if(!initWindow(window_size)) {
    throw std::runtime_error("Failed to create GLFW window");
  } else {
    LOG("GLFW window created!\n");
  }
}

WGPUGLFWWindow::WGPUGLFWWindow(window_size_t window_size) {
  window_size = window_size;
  if(!initWindow(window_size)) {
    throw std::runtime_error("Failed to create GLFW window");
  } else {
    LOG("GLFW window created!\n");
  }
}

window_size_t& WGPUGLFWWindow::getWindowSize() {
  return window_size;
}

GLFWwindow* WGPUGLFWWindow::getWGPUGLFWWindow() {
  return window;
}

void WGPUGLFWWindow::Terminate() {
  glfwDestroyWindow(window);
	glfwTerminate();
}

WGPUGLFWWindow::~WGPUGLFWWindow() {
  if(window)Terminate();
}

}