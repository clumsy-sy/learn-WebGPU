#include "application.h"

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
	wgpu::InstanceDescriptor instanceDescriptor = {};
	wgpu::Instance instance = wgpu::createInstance(instanceDescriptor);
	
	// 获取适配器
	LOG("Requesting adapter...\n");
	surface = glfwGetWGPUSurface(instance, window); // <--  glfw surface 获取
	
	wgpu::RequestAdapterOptions adapterOpts = {};
	adapterOpts.nextInChain = nullptr;
	adapterOpts.compatibleSurface = surface; // <--  adapter 表面
	
	wgpu::Adapter adapter = instance.requestAdapter(adapterOpts);
	LOG("Got adapter: %p", static_cast<void*>(&adapter));
	
	// 不在需要 instance，释放
	instance.release();
  // end 1.0 -----
	
  // begin 2.0 -----
	// 获取 device
	LOG("Requesting device...\n");
	wgpu::DeviceDescriptor deviceDesc = {};
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
	device = adapter.requestDevice(deviceDesc);
	LOG("Got device: %p", static_cast<void*>(&device));
	
	// Device error 回调
	uncapturedErrorCallbackHandle = device.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
		std::cout << "Uncaptured device error: type " << type;
		if (message) std::cout << " (" << message << ")";
		std::cout << '\n';
	});
  // end 2.0 -----
	
  // begin 3.0 -----
  // 获取 队列
	queue = wgpuDeviceGetQueue(device);
	LOG("Got queue: %p", static_cast<void*>(&queue));
  // 配置 surface
  wgpu::SurfaceConfiguration config = {};
  config.nextInChain = nullptr;

  //为底层交换链创建的纹理配置
  config.width = window_size.width;
  config.height = window_size.height;
  config.usage = wgpu::TextureUsage::RenderAttachment;
  wgpu::TextureFormat surfaceFormat = surface.getPreferredFormat(adapter);
  config.format = surfaceFormat;

  // 不需要任何特定的视图格式
	config.viewFormatCount = 0;
	config.viewFormats = nullptr;
	config.device = device;
	config.presentMode = wgpu::PresentMode::Fifo;
	config.alphaMode = wgpu::CompositeAlphaMode::Auto;

	surface.configure(config);

  // 充分利用适配器后释放适配器
	adapter.release();

  // end 3.0 -----
	return true;
}

void Application::Terminate() {
	// Move all the release/destroy/terminate calls here
	surface.unconfigure();
	queue.release();
	surface.release();
	device.release();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::MainLoop() {
	glfwPollEvents();

  // 获取 view
  wgpu::TextureView targetView = GetNextSurfaceTextureView();
  if(!targetView) {
    throw std::runtime_error("Failed to get next surface texture view");
    return;
  }

  // 创建 command encoder
  wgpu::CommandEncoderDescriptor encoderDesc = {};
  encoderDesc.nextInChain = nullptr;
  encoderDesc.label = "Command encoder";
  wgpu::CommandEncoder encoder = device.createCommandEncoder(encoderDesc);

  // 创建 render pass 描述符
  wgpu::RenderPassDescriptor renderPassDesc = {};
  renderPassDesc.nextInChain = nullptr;

  // 渲染管道描述符的 attachment 描述了通道的目标纹理
  wgpu::RenderPassColorAttachment colorAttachment = {};
  colorAttachment.view = targetView;
  colorAttachment.resolveTarget = nullptr;
  colorAttachment.loadOp = wgpu::LoadOp::Clear;
  colorAttachment.storeOp = wgpu::StoreOp::Store;
  colorAttachment.clearValue = wgpu::Color{0.9, 0.1, 0.2, 1.0 };
#ifndef WEBGPU_BACKEND_WGPU
	colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU
  renderPassDesc.colorAttachmentCount = 1;
  renderPassDesc.colorAttachments = &colorAttachment;
  renderPassDesc.depthStencilAttachment = nullptr;
  renderPassDesc.timestampWrites = nullptr;

  // 创建 render pass
  wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
  renderPass.end();
  renderPass.release(); // <--  释放 render pass

  // 执行 encoder 并且提交
  wgpu::CommandBufferDescriptor cmdBufferDesacriptor = {};
  cmdBufferDesacriptor.nextInChain = nullptr;
  cmdBufferDesacriptor.label = "Command buffer Descriptor";

  wgpu::CommandBuffer command = encoder.finish(cmdBufferDesacriptor);
  wgpuCommandEncoderRelease(encoder); // <--  释放 encoder

	LOG("Submitting command...\n");
	queue.submit(1, &command);
	command.release();
	LOG("Command submitted.\n");

	// At the end of the frame
	targetView.release();

#ifndef __EMSCRIPTEN__
	surface.present();
#endif

#if defined(WEBGPU_BACKEND_DAWN)
	device.tick();
#elif defined(WEBGPU_BACKEND_WGPU)
	device.poll(false);
#endif
}


bool Application::IsRunning() {
	return !glfwWindowShouldClose(window);
}

wgpu::TextureView Application::GetNextSurfaceTextureView(){
  // 获取当前的表面纹理
  wgpu::SurfaceTexture surfaceTexture;
  wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
  if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
    throw std::runtime_error("Failed to get current texture");
		// return nullptr;
	}
  // 创建一个视图
  wgpu::TextureViewDescriptor viewDescriptor = {};
  viewDescriptor.nextInChain = nullptr;
  viewDescriptor.label = "Surface texture view";
  viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
  viewDescriptor.dimension = wgpu::TextureViewDimension::_2D;
	viewDescriptor.baseMipLevel = 0;
	viewDescriptor.mipLevelCount = 1;
	viewDescriptor.baseArrayLayer = 0;
	viewDescriptor.arrayLayerCount = 1;
	viewDescriptor.aspect = wgpu::TextureAspect::All;
	wgpu::TextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

  return targetView;
}

}
