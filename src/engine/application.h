#pragma once

#include "../utils/global.h"
#include "../utils/data-structure.h"
#include "../utils/utils.h"

#include "glfw-window.h"


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
	/**
		* @brief 获取下一个可用的纹理视图
		* @return 纹理
		*/
	wgpu::TextureView GetNextSurfaceTextureView();

	/**
		* @brief 初始化实例
		*/
	void InitInstance();
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
	WGPUGLFWWindow wgpuGLFWWindow;

	// WebGPU device
	wgpu::Device device = nullptr;
	// grraphics or computer queue
	wgpu::Queue queue = nullptr;
	// surface 对象
	wgpu::Surface surface = nullptr;
	// 错误回调
	std::unique_ptr<wgpu::ErrorCallback> uncapturedErrorCallbackHandle;
	// 着色器模块
	wgpu::ShaderModule shaderModule = nullptr;
	// 渲染格式
	// wgpu::TextureFormat surfaceFormat = wgpu::TextureFormat::Undefined;
	wgpu::TextureFormat swapChainFormat = wgpu::TextureFormat::Undefined;
	// 渲染管线
	wgpu::RenderPipeline pipeline = nullptr;
	// 交换链
	wgpu::SwapChain swapChain = nullptr;
	// 顶点缓冲区
	wgpu::Buffer vertexBuffer = nullptr;
	// 索引缓冲区
	std::vector<VertexAttributes> vertexData;
	// 深度纹理
	wgpu::Texture depthTexture = nullptr;
	// 深度纹理视图
	wgpu::TextureView depthTextureView = nullptr;
	// uniform
	Uniform uniform = {};
	// uniform buffer
	wgpu::Buffer uniformBuffer = nullptr;
	// bind group
	wgpu::BindGroup bindGroup = nullptr;
	// 着色器代码
	std::string shaderCodeFilePath = "C:/Users/Sy200/Desktop/learn-WebGPU/src/shader/base.wgsl";
	// ojb 地址
	std::string objFilePath = "C:/Users/Sy200/Desktop/learn-WebGPU/resources/pyramid.obj";
	// std::string objFilePath = "C:/Users/Sy200/Desktop/learn-WebGPU/resources/bunny/bunny.obj";
};

  
}