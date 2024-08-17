#pragma once

#include <cstddef>
#include <memory>
#include <iostream>


#include <webgpu/webgpu.h>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#include "../utils/webgpu-utils.h"


namespace webgpu {


class Application {
public:
    // 获取实例
    static std::unique_ptr<Application>& GetInstance() {
			static std::unique_ptr<Application> instance(new Application());
			return instance;
		}

		Application(const Application&) = delete; // 删除拷贝构造函数
		Application& operator=(const Application&) = delete; // 删除赋值运算符
		~Application() {}; // 公有析构函数

    /**
		 * @brief 初始化
		 * @return true
		 */
    bool Initialize();

    /**
		 * @brief 销毁
		 */
    void Terminate();

		/**
		 * @brief 主循环，绘制框架并处理事件
		 */
    void MainLoop();

    /**
		 * @brief 只要主循环继续运行，就会返回true
		 * @return true
		 */
    bool IsRunning();

private:
    Application() {} // Private constructor

    // 初始化和主循环之间共享的所有变量放在这里
    GLFWwindow* window = nullptr;
    WGPUDevice device;
    WGPUQueue queue;
    WGPUSurface surface;
};

  
}