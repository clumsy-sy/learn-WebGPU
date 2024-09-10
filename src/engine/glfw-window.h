#include "../utils/global.h"
#include "src/utils/data-structure.h"


namespace webgpu {

class WGPUGLFWWindow {
  public:
  /**
   * @brief 构造函数
   */
  WGPUGLFWWindow();

  /**
   * @brief 构造函数
   * @param window_size 窗口大小
   */
  WGPUGLFWWindow(window_size_t window_size);

  /**
   * @brief 获取窗口大小
   */
  window_size_t& getWindowSize();

  /**
   * @brief 获取GLFW窗口
   */
  GLFWwindow* getWGPUGLFWWindow();

  /**
   * @brief 销毁窗口
   */
  void Terminate();

  ~WGPUGLFWWindow();

public:
  // 窗口大小
  window_size_t window_size = { 800, 600 };
  // GLFW 窗口
	GLFWwindow* window = nullptr;
private:

  bool initWindow(window_size_t window_size);

};

}