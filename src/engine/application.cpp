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
	LOG("Got device: %p\n", static_cast<void*>(&device));
	
	// Device error 回调
	uncapturedErrorCallbackHandle = device.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
		std::cout << "Uncaptured device error: type " << type;
		if (message) std::cout << " (" << message << ")";
		std::cout << '\n';
	});
  // end 2.0 -----
	
  // begin 3.0 -----
  // 获取 队列
	queue = device.getQueue();
	LOG("Got queue: %p\n", static_cast<void*>(&queue));
  // 配置 surface
  wgpu::SurfaceConfiguration config = {};
  config.nextInChain = nullptr;

  //为底层交换链创建的纹理配置
  config.width = window_size.width;
  config.height = window_size.height;
  config.usage = wgpu::TextureUsage::RenderAttachment;
  surfaceFormat = surface.getPreferredFormat(adapter);
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

	InitializePipeline();

  // end 3.0 -----
	return true;
}

void Application::Terminate() {
	// Move all the release/destroy/terminate calls here
	pipeline.release();
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
  encoderDesc.label = "Command encoder";
  wgpu::CommandEncoder encoder = device.createCommandEncoder(encoderDesc);

  // 创建 render pass 描述符
  wgpu::RenderPassDescriptor renderPassDesc = {};

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
  
	// 选择使用的 pipeline
	renderPass.setPipeline(pipeline);
	// Draw 1 instance of a 3-vertices shape
	renderPass.draw(3, 1, 0, 0);

	renderPass.end();
	renderPass.release();

  // 执行 encoder 并且提交
  wgpu::CommandBufferDescriptor cmdBufferDesacriptor = {};
  cmdBufferDesacriptor.nextInChain = nullptr;
  cmdBufferDesacriptor.label = "Command buffer Descriptor";

  wgpu::CommandBuffer command = encoder.finish(cmdBufferDesacriptor);
  wgpuCommandEncoderRelease(encoder); // <--  释放 encoder

	// LOG("Submitting command...\n");
	queue.submit(1, &command);
	command.release();
	// LOG("Command submitted.\n");

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

void Application::InitializePipeline(){
	// 加载着色器模块
	wgpu::ShaderModuleDescriptor shaderDesc;
#ifdef WEBGPU_BACKEND_WGPU
	shaderDesc.hintCount = 0;
	shaderDesc.hints = nullptr;
#endif

	// 使用扩展机制来指定着色器模块描述符中的WGSL部分
	wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
	// 设置链表头
	shaderCodeDesc.chain.next = nullptr;
	shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
	// 连接链表
	shaderDesc.nextInChain = &shaderCodeDesc.chain;
	// 从文件中读取着色器源码
	auto shaderSourceCode = ReadShaderFile(shaderCode);
		// std::cout << "shader code :" << shaderSourceCode << std::endl;
	shaderCodeDesc.code = shaderSourceCode.c_str();
	wgpu::ShaderModule shaderModule = device.createShaderModule(shaderDesc);

	// 创建渲染管线
	wgpu::RenderPipelineDescriptor pipelineDesc;

	// 对于这个简单的例子，我们不使用顶点缓冲区
	pipelineDesc.vertex.bufferCount = 0;
	pipelineDesc.vertex.buffers = nullptr;

	// 指定可编程顶点着色器阶段由着色器模块中的 `vs_main` 函数描述
	pipelineDesc.vertex.module = shaderModule;
	pipelineDesc.vertex.entryPoint = "vs_main";
	pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;

	// 每三个顶点构成一个三角形
	pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	
	// 未指定顶点连接顺序时，默认按顺序连接顶点
	pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
	
	// 当从正面观察时，假设面的顶点按照逆时针(CCW)顺序排列
	pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
	
	// 由于我们不裁剪指向我们的背面，因此面的方向并不重要
	pipelineDesc.primitive.cullMode = wgpu::CullMode::None;

	// 指定可编程片段着色器阶段由着色器模块中的 `fs_main` 函数描述
	wgpu::FragmentState fragmentState;
	fragmentState.module = shaderModule;
	fragmentState.entryPoint = "fs_main";
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;

	// 设置混合状态
	wgpu::BlendState blendState;
	blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
	blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
	blendState.color.operation = wgpu::BlendOperation::Add;
	blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
	blendState.alpha.dstFactor = wgpu::BlendFactor::One;
	blendState.alpha.operation = wgpu::BlendOperation::Add;
	
	// 颜色目标状态
	wgpu::ColorTargetState colorTarget;
	colorTarget.format = surfaceFormat;
	colorTarget.blend = &blendState;
	colorTarget.writeMask = wgpu::ColorWriteMask::All; // 可以选择性地写入颜色通道
	
	// 我们的渲染通道只有一个输出颜色附件
	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;
	pipelineDesc.fragment = &fragmentState;

	// 目前不使用深度/模板测试
	pipelineDesc.depthStencil = nullptr;

	// 每像素采样数
	pipelineDesc.multisample.count = 1;

	// 默认掩码值，表示“所有位都开启”
	pipelineDesc.multisample.mask = ~0u;

	// 默认值（对于 count = 1 时无关紧要）
	pipelineDesc.multisample.alphaToCoverageEnabled = false;
	pipelineDesc.layout = nullptr;
	
	// 创建渲染管线
	pipeline = device.createRenderPipeline(pipelineDesc);

	// 不再需要访问着色器模块
	shaderModule.release();
}

std::string Application::ReadShaderFile(const std::string& filePath){
	std::ifstream file(filePath.c_str());

	if (!file) {
		throw std::runtime_error("无法打开文件 " + filePath + "\n");
	}

	// 获取文件内容
	std::string shaderSource((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	return shaderSource;
}

}
