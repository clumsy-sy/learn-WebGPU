#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <iostream>


#include <utility>
#include <webgpu/webgpu.h>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#include "../utils/webgpu-utils.h"
#include "../utils/data-structure.h"


namespace webgpu {

static window_size_t window_size = { 800, 600 };

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
	WGPUTextureView GetNextSurfaceTextureView();

private:
	Application() {} // Private constructor

	// 初始化和主循环之间共享的所有变量放在这里

	// GLFW 窗口
	GLFWwindow* window = nullptr;
	// WebGPU device
	WGPUDevice device;
	// grraphics or computer queue
	WGPUQueue queue;
	// surface 对象
	WGPUSurface surface;
};

  
}