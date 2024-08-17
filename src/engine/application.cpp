#include "application.h"
#include <stdexcept>

namespace webgpu {

bool Application::Initialize() {
  // begin 0.0 -----
	// glfw 初始化
	glfwInit(); // <--  glfw 初始化
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // <--  glfwCreateWindow 额外信息
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // <--  glfwWindow 大小不可改变
	window = glfwCreateWindow(window_size.width, window_size.height, "Learn WebGPU", nullptr, nullptr); // <--  glfwCreateWindow 创建窗口
	// end 0.0 -----

  // begin 1.0 -----
	// 创建实例
	WGPUInstance instance = wgpuCreateInstance(nullptr);
	
	// 获取适配器
	std::cout << "Requesting adapter...\n";
	surface = glfwGetWGPUSurface(instance, window); // <--  glfw surface 获取
	
	WGPURequestAdapterOptions adapterOpts = {};
	adapterOpts.nextInChain = nullptr;
	adapterOpts.compatibleSurface = surface; // <--  adapter 表面
	
	WGPUAdapter adapter = requestAdapterSync(instance, &adapterOpts);
	std::cout << "Got adapter: " << adapter << '\n';
	
	// 不在需要 instance，释放
	wgpuInstanceRelease(instance);
  // end 1.0 -----
	
  // begin 2.0 -----
	// 获取 device
	std::cout << "Requesting device..." << '\n';
	WGPUDeviceDescriptor deviceDesc = {};
	deviceDesc.nextInChain = nullptr;
	deviceDesc.label = "Physical Device"; // <--  随便命名
	deviceDesc.requiredFeatureCount = 0; // <--  没有特殊需求
	deviceDesc.requiredLimits = nullptr; // <--  没有特殊限制
	deviceDesc.defaultQueue.nextInChain = nullptr;
	deviceDesc.defaultQueue.label = "The default queue";

	// 当设备停止时调用的函数。
	deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */) {
		std::cout << "Device lost: reason " << reason;
		if (message) std::cout << " (" << message << ")";
		std::cout << '\n';
	};
	device = requestDeviceSync(adapter, &deviceDesc);
	std::cout << "Got device: " << device << '\n';
	
	// Device error 回调
	auto onDeviceError = [](WGPUErrorType type, char const* message, void* /* pUserData */) {
		std::cout << "Uncaptured device error: type " << type;
		if (message) std::cout << " (" << message << ")";
		std::cout << '\n';
	};
	wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr /* pUserData */);
  // end 2.0 -----
	
  // begin 3.0 -----
  // 获取 队列
	queue = wgpuDeviceGetQueue(device);
  std::cout << "Got queue: " << queue << '\n';
  // 配置 surface
  WGPUSurfaceConfiguration config = {};
  config.nextInChain = nullptr;

  //为底层交换链创建的纹理配置
  config.width = window_size.width;
  config.height = window_size.height;
  config.usage = WGPUTextureUsage_RenderAttachment;
  WGPUTextureFormat surfaceFormat = wgpuSurfaceGetPreferredFormat(surface, adapter);
  config.format = surfaceFormat;

  // 不需要任何特定的视图格式
	config.viewFormatCount = 0;
	config.viewFormats = nullptr;
	config.device = device;
	config.presentMode = WGPUPresentMode_Fifo;
	config.alphaMode = WGPUCompositeAlphaMode_Auto;

	wgpuSurfaceConfigure(surface, &config);

  // 充分利用适配器后释放适配器
	wgpuAdapterRelease(adapter);

  // end 3.0 -----
	return true;
}

void Application::Terminate() {
	// Move all the release/destroy/terminate calls here
	wgpuQueueRelease(queue);
	wgpuSurfaceRelease(surface);
	wgpuDeviceRelease(device);
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::MainLoop() {
	glfwPollEvents();

  // 获取 view
  WGPUTextureView targetView = GetNextSurfaceTextureView();
  if(!targetView) {
    throw std::runtime_error("Failed to get next surface texture view");
    return;
  }

  // 创建 command encoder
  WGPUCommandEncoderDescriptor encoderDesc = {};
  encoderDesc.nextInChain = nullptr;
  encoderDesc.label = "Command encoder";
  WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

  // 创建 render pass 描述符
  WGPURenderPassDescriptor renderPassDesc = {};
  renderPassDesc.nextInChain = nullptr;

  // 渲染管道描述符的 attachment 描述了通道的目标纹理
  WGPURenderPassColorAttachment colorAttachment = {};
  colorAttachment.view = targetView;
  colorAttachment.resolveTarget = nullptr;
  colorAttachment.loadOp = WGPULoadOp_Clear;
  colorAttachment.storeOp = WGPUStoreOp_Store;
  colorAttachment.clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0 };
#ifndef WEBGPU_BACKEND_WGPU
	colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU
  renderPassDesc.colorAttachmentCount = 1;
  renderPassDesc.colorAttachments = &colorAttachment;
  renderPassDesc.depthStencilAttachment = nullptr;
  renderPassDesc.timestampWrites = nullptr;

  // 创建 render pass
  WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
  wgpuRenderPassEncoderEnd(renderPass);
  wgpuRenderPassEncoderRelease(renderPass); // <--  释放 render pass

  // 执行 encoder 并且提交
  WGPUCommandBufferDescriptor cmdBufferDesacriptor = {};
  cmdBufferDesacriptor.nextInChain = nullptr;
  cmdBufferDesacriptor.label = "Command buffer Descriptor";

  WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDesacriptor);
  wgpuCommandEncoderRelease(encoder); // <--  释放 encoder

  std::cout << "Submitting command...\n";
	wgpuQueueSubmit(queue, 1, &command);
	wgpuCommandBufferRelease(command);
	std::cout << "Command submitted.\n";

	// At the end of the frame
	wgpuTextureViewRelease(targetView);

#ifndef __EMSCRIPTEN__
	wgpuSurfacePresent(surface);
#endif

#if defined(WEBGPU_BACKEND_DAWN)
	wgpuDeviceTick(device);
#elif defined(WEBGPU_BACKEND_WGPU)
	wgpuDevicePoll(device, false, nullptr);
#endif
}


bool Application::IsRunning() {
	return !glfwWindowShouldClose(window);
}

WGPUTextureView Application::GetNextSurfaceTextureView(){
  // 获取当前的表面纹理
  WGPUSurfaceTexture surfaceTexture;
  wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
    throw std::runtime_error("Failed to get current texture");
		// return nullptr;
	}
  // 创建一个视图
  WGPUTextureViewDescriptor viewDescriptor = {};
  viewDescriptor.nextInChain = nullptr;
  viewDescriptor.label = "Surface texture view";
  viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
  viewDescriptor.dimension = WGPUTextureViewDimension_2D;
	viewDescriptor.baseMipLevel = 0;
	viewDescriptor.mipLevelCount = 1;
	viewDescriptor.baseArrayLayer = 0;
	viewDescriptor.arrayLayerCount = 1;
	viewDescriptor.aspect = WGPUTextureAspect_All;
	WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

  return targetView;
}

}
