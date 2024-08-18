#pragma once

#include "../utils/global.h"
#include "../utils/data-structure.h"
#include <string>


namespace webgpu {

class Application {
public:
	// 窗口大小
	window_size_t window_size = { 800, 600 };
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
	/**
		* @brief 获取下一个可用的纹理视图
		* @return 纹理
		*/
	wgpu::TextureView GetNextSurfaceTextureView();
	/**
		* @brief 初始化管线
		*/
	void InitializePipeline();

	/**
		* @brief 读取着色器文件
 		*/
	std::string ReadShaderFile(const std::string& filePath);

private:
	Application() {} // Private constructor

	// 初始化和主循环之间共享的所有变量放在这里

	// GLFW 窗口
	GLFWwindow* window = nullptr;
	// WebGPU device
	wgpu::Device device = nullptr;
	// grraphics or computer queue
	wgpu::Queue queue = nullptr;
	// surface 对象
	wgpu::Surface surface = nullptr;
	// 错误回调
	std::unique_ptr<wgpu::ErrorCallback> uncapturedErrorCallbackHandle;
	// 渲染格式
	wgpu::TextureFormat surfaceFormat = wgpu::TextureFormat::Undefined;
	// 渲染管线
	wgpu::RenderPipeline pipeline = nullptr;
	// 着色器代码
	std::string shaderCode = "C:/Users/Sy200/Desktop/learn-WebGPU/src/shader/base.wgsl";
};

  
}