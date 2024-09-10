#include "application.h"
#include "src/utils/global.h"

namespace webgpu {

bool Application::Initialize() {
	// glfw 初始化

  InitInstance();

	InitializePipeline();

	return true;
}

void Application::InitInstance(){
	// begin 1.0 -----
	// 创建实例
	wgpu::InstanceDescriptor instanceDescriptor = {};
	wgpu::Instance instance = wgpu::createInstance(instanceDescriptor);
	checkNullPointerError(instance, "instance");
	
	// 获取适配器
	LOG("Requesting adapter...\n");
	surface = glfwGetWGPUSurface(instance, wgpuGLFWWindow.window); // <--  glfw surface 获取
	
	wgpu::RequestAdapterOptions adapterOpts = {};
	adapterOpts.nextInChain = nullptr;
	adapterOpts.compatibleSurface = surface; // <--  adapter 表面
	
	wgpu::Adapter adapter = instance.requestAdapter(adapterOpts);
	LOG("Got adapter: %p", static_cast<void*>(&adapter));

	wgpu::SupportedLimits supportedLimits;
	adapter.getLimits(&supportedLimits);

	// 不在需要 instance，释放
	// instance.release();
  // end 1.0 -----
	
  // begin 2.0 -----
	// 配置 device 的限制
	// 获取 device
	LOG("Requesting device...\n");
	wgpu::RequiredLimits requiredLimits = wgpu::Default;
	requiredLimits.limits.maxVertexAttributes = 3;
	requiredLimits.limits.maxVertexBuffers = 1;
	requiredLimits.limits.maxBufferSize = 100000 * sizeof(VertexAttributes);
	requiredLimits.limits.maxVertexBufferArrayStride = sizeof(VertexAttributes);
	requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
	requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
	requiredLimits.limits.maxInterStageShaderComponents = 6;
	requiredLimits.limits.maxBindGroups = 1;
	requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
	requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);
	requiredLimits.limits.maxTextureDimension1D = wgpuGLFWWindow.window_size.width;
	requiredLimits.limits.maxTextureDimension2D = wgpuGLFWWindow.window_size.height;
	requiredLimits.limits.maxTextureArrayLayers = 1;
	
	wgpu::DeviceDescriptor deviceDesc = {};
	deviceDesc.nextInChain = nullptr;
	deviceDesc.label = "Physical Device"; // <--  随便命名
	deviceDesc.requiredFeatureCount = 0; // <--  没有特殊需求
	deviceDesc.requiredLimits = &requiredLimits; // <--  限制条件
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

	// 创建 swapchain
	LOG("Creating swapchain...\n");
#ifdef WEBGPU_BACKEND_WGPU
	swapChainFormat = surface.getPreferredFormat(adapter);
#else
	swapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
#endif
	if(swapChainFormat == wgpu::TextureFormat::Undefined){
		std::cerr << "surface.getPreferredFormat(adapter) == wgpu::TextureFormat::Undefined" << '\n';
	}
	wgpu::SwapChainDescriptor swapChainDesc;
	swapChainDesc.width = wgpuGLFWWindow.window_size.width;
	swapChainDesc.height = wgpuGLFWWindow.window_size.height;
	swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
	swapChainDesc.format = swapChainFormat;
	swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
	swapChain = device.createSwapChain(surface, swapChainDesc);
	LOG("Swapchain %p\n", static_cast<void*>(&swapChain));
	LOG("Creating shader module...\n");
	shaderModule = loadShaderModule(shaderCodeFilePath, device);
	LOG("shader module %p\n", static_cast<void*>(&shaderModule));

  // // 配置 surface
  // wgpu::SurfaceConfiguration config = {};
  // config.nextInChain = nullptr;

  // //为底层交换链创建的纹理配置
  // config.width = wgpuGLFWWindow.window_size.width;
  // config.height = wgpuGLFWWindow.window_size.height;
  // config.usage = wgpu::TextureUsage::RenderAttachment;
  // surfaceFormat = surface.getPreferredFormat(adapter);
  // config.format = surfaceFormat;

  // // 不需要任何特定的视图格式
	// config.viewFormatCount = 0;
	// config.viewFormats = nullptr;
	// config.device = device;
	// config.presentMode = wgpu::PresentMode::Fifo;
	// config.alphaMode = wgpu::CompositeAlphaMode::Auto;

	// surface.configure(config);

  // // 充分利用适配器后释放适配器
	// adapter.release();
}

void Application::InitializePipeline(){
	// 加载着色器模块
// 	wgpu::ShaderModuleDescriptor shaderDesc;
// #ifdef WEBGPU_BACKEND_WGPU
// 	shaderDesc.hintCount = 0;
// 	shaderDesc.hints = nullptr;
// #endif

// 	// 使用扩展机制来指定着色器模块描述符中的WGSL部分
// 	wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
// 	// 设置链表头
// 	shaderCodeDesc.chain.next = nullptr;
// 	shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
// 	// 连接链表
// 	shaderDesc.nextInChain = &shaderCodeDesc.chain;
// 	// 从文件中读取着色器源码
// 	auto shaderSourceCode = ReadShaderFile(shaderCode);
// 	shaderCodeDesc.code = shaderSourceCode.c_str();
// 	wgpu::ShaderModule shaderModule = device.createShaderModule(shaderDesc);

	LOG("Creating render pipeline...\n");
	// 创建渲染管线
	wgpu::RenderPipelineDescriptor pipelineDesc = {};

	// 获取顶点属性
	std::vector<wgpu::VertexAttribute> vertexAttribs(3);

	// 位置属性
	vertexAttribs[0].shaderLocation = 0;
	vertexAttribs[0].format = wgpu::VertexFormat::Float32x3;
	vertexAttribs[0].offset = 0;

	// 法向量属性
	vertexAttribs[1].shaderLocation = 1;
	vertexAttribs[1].format = wgpu::VertexFormat::Float32x3;
	vertexAttribs[1].offset = offsetof(VertexAttributes, normal);

	// 颜色属性
	vertexAttribs[2].shaderLocation = 2;
	vertexAttribs[2].format = wgpu::VertexFormat::Float32x3;
	vertexAttribs[2].offset = offsetof(VertexAttributes, color);

	wgpu::VertexBufferLayout vertexBufferLayout = {};
	vertexBufferLayout.attributeCount = (uint32_t)vertexAttribs.size();
	vertexBufferLayout.attributes = vertexAttribs.data();
	vertexBufferLayout.arrayStride = sizeof(VertexAttributes);

	vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

	// 使用一个顶点缓冲区
	pipelineDesc.vertex.bufferCount = 1;
	pipelineDesc.vertex.buffers = &vertexBufferLayout;

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
	pipelineDesc.fragment = &fragmentState;

	// 设置混合状态
	wgpu::BlendState blendState{};
	blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
	blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
	blendState.color.operation = wgpu::BlendOperation::Add;
	blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
	blendState.alpha.dstFactor = wgpu::BlendFactor::One;
	blendState.alpha.operation = wgpu::BlendOperation::Add;
	
	// 颜色目标状态
	wgpu::ColorTargetState colorTarget;
	colorTarget.format = swapChainFormat;
	colorTarget.blend = &blendState;
	colorTarget.writeMask = wgpu::ColorWriteMask::All; // 可以选择性地写入颜色通道
	
	// 我们的渲染通道只有一个输出颜色附件
	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;

	// 深度状态
	wgpu::DepthStencilState depthStencilState = wgpu::Default;
	depthStencilState.depthCompare = wgpu::CompareFunction::Less;
	depthStencilState.depthWriteEnabled = true;
	wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
	depthStencilState.format = depthTextureFormat;
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;
	// 深度/模板测试
	pipelineDesc.depthStencil = &depthStencilState;

	// 每像素采样数
	pipelineDesc.multisample.count = 1;

	// 默认掩码值，表示“所有位都开启”
	pipelineDesc.multisample.mask = ~0u;

	// 默认值（对于 count = 1 时无关紧要）
	pipelineDesc.multisample.alphaToCoverageEnabled = false;

	// 创建绑定布局
	wgpu::BindGroupLayoutEntry bindingLayout = wgpu::Default;
	bindingLayout.binding = 0;
	bindingLayout.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
	bindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
	bindingLayout.buffer.minBindingSize = sizeof(Uniform);

	// 创建一个绑定布局
	wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{};
	bindGroupLayoutDesc.entryCount = 1;
	bindGroupLayoutDesc.entries = &bindingLayout;
	wgpu::BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

	// 创建管线布局
	wgpu::PipelineLayoutDescriptor layoutDesc{};
	layoutDesc.bindGroupLayoutCount = 1;
	layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
	wgpu::PipelineLayout layout = device.createPipelineLayout(layoutDesc);
	pipelineDesc.layout = layout;
	
	// 创建渲染管线
	pipeline = device.createRenderPipeline(pipelineDesc);
	LOG("Render pipeline %p\n", static_cast<void*>(&pipeline));

	// 创建深度纹理
	wgpu::TextureDescriptor depthTextureDesc;
	depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
	depthTextureDesc.format = depthTextureFormat;
	depthTextureDesc.mipLevelCount = 1;
	depthTextureDesc.sampleCount = 1;
	depthTextureDesc.size = {wgpuGLFWWindow.window_size.width, wgpuGLFWWindow.window_size.height, 1};
	depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
	depthTextureDesc.viewFormatCount = 1;
	depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
	depthTexture = device.createTexture(depthTextureDesc);
	LOG("Depth texture %p\n", static_cast<void*>(&depthTexture));

	// 创建深度纹理视图（由栅格化器生成）
	wgpu::TextureViewDescriptor depthTextureViewDesc;
	depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
	depthTextureViewDesc.baseArrayLayer = 0;
	depthTextureViewDesc.arrayLayerCount = 1;
	depthTextureViewDesc.baseMipLevel = 0;
	depthTextureViewDesc.mipLevelCount = 1;
	depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
	depthTextureViewDesc.format = depthTextureFormat;
	depthTextureView = depthTexture.createView(depthTextureViewDesc);
	LOG("Depth texture view %p\n", static_cast<void*>(&depthTextureView));

	// 创建数据
	std::vector<float> pointData;
	std::vector<uint16_t> indexData;

	// Load mesh data from OBJ file

	bool success = loadGeometryFromObj(objFilePath, vertexData);
	if (!success) {
		std::cerr << "Could not load geometry!" << '\n';
		throw std::runtime_error("Could not load geometry!");
	}

	// 创建顶点缓冲区
	wgpu::BufferDescriptor bufferDesc;
	bufferDesc.size = vertexData.size() * sizeof(VertexAttributes); // changed
	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
	bufferDesc.mappedAtCreation = false;
	vertexBuffer = device.createBuffer(bufferDesc);
	queue.writeBuffer(vertexBuffer, 0, vertexData.data(), bufferDesc.size); // changed

	// int indexCount = static_cast<int>(vertexData.size()); // changed
	
	// 创建uniform缓冲区
	bufferDesc.size = sizeof(Uniform);
	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
	bufferDesc.mappedAtCreation = false;
	uniformBuffer = device.createBuffer(bufferDesc);
	checkNullPointerError(uniformBuffer, "uniformBuffer");
	// 上传初始uniform数据
	

	// 构建转移矩阵

	// Translate the view
	glm::vec3 focalPoint(0.0, 0.0, -1.0);
	// Rotate the object
	float angle1 = 2.0f; // arbitrary time
	// Rotate the view point
	float angle2 = 3.0f * PI / 4.0f;

	glm::mat4x4 S = glm::scale(glm::mat4x4(1.0), glm::vec3(0.3f));
	glm::mat4x4 T1 = glm::mat4x4(1.0);
	glm::mat4x4 R1 = glm::rotate(glm::mat4x4(1.0), angle1, glm::vec3(0.0, 0.0, 1.0));
	uniform.modelMatrix = R1 * T1 * S;

	glm::mat4x4 R2 = glm::rotate(glm::mat4x4(1.0), -angle2, glm::vec3(1.0, 0.0, 0.0));
	glm::mat4x4 T2 = glm::translate(glm::mat4x4(1.0), -focalPoint);
	uniform.viewMatrix = T2 * R2;

	float ratio = (float)wgpuGLFWWindow.window_size.width / (float)wgpuGLFWWindow.window_size.height;
	float focalLength = 2.0;
	float near = 0.01f;
	float far = 100.0f;
	float divider = 1 / (focalLength * (far - near));
	uniform.projectionMatrix = transpose(glm::mat4x4(
		1.0, 0.0, 0.0, 0.0,
		0.0, ratio, 0.0, 0.0,
		0.0, 0.0, far * divider, -far * near * divider,
		0.0, 0.0, 1.0 / focalLength, 0.0
	));

	uniform.time = 1.0f;
	uniform.color = { 0.0f, 1.0f, 0.4f, 1.0f };
	queue.writeBuffer(uniformBuffer, 0, &uniform, sizeof(Uniform));

	// Create a binding
	wgpu::BindGroupEntry binding{};
	binding.binding = 0;
	binding.buffer = uniformBuffer;
	binding.offset = 0;
	binding.size = sizeof(Uniform);

	// A bind group contains one or multiple bindings
	wgpu::BindGroupDescriptor bindGroupDesc;
	bindGroupDesc.layout = bindGroupLayout;
	bindGroupDesc.entryCount = bindGroupLayoutDesc.entryCount;
	bindGroupDesc.entries = &binding;
	bindGroup = device.createBindGroup(bindGroupDesc);
}

void Application::Terminate() {
	// Move all the release/destroy/terminate calls here
	vertexBuffer.destroy();
	vertexBuffer.release();

	depthTextureView.release();
	depthTexture.destroy();
	depthTexture.release();

	pipeline.release();
	shaderModule.release();
	swapChain.release();
	surface.unconfigure();
	queue.release();
	surface.release();
	device.release();

	wgpuGLFWWindow.Terminate();
}

void Application::MainLoop() {
	glfwPollEvents();
	// LOG("Main loop begin\n");

	// 更新 uniform buffer
	uniform.time = static_cast<float>(glfwGetTime()); // glfwGetTime returns a double
	// 仅更新 uniformBuffer 的第一个 float
	queue.writeBuffer(uniformBuffer, offsetof(Uniform, time), &uniform.time, sizeof(Uniform::time));

	// 更新视角矩阵
	float angle1 = uniform.time;
	glm::mat4x4 R1 = glm::rotate(glm::mat4x4(1.0), angle1, glm::vec3(0.0, 0.0, 1.0));
	glm::mat4x4 T1 = glm::mat4x4(1.0);
	glm::mat4x4 S = glm::scale(glm::mat4x4(1.0), glm::vec3(0.3f));
	uniform.modelMatrix = R1 * T1 * S;
	queue.writeBuffer(uniformBuffer, offsetof(Uniform, modelMatrix), &uniform.modelMatrix, sizeof(Uniform::modelMatrix));

	// std::cout << uniform << "\n";
  // 获取 view
  wgpu::TextureView nextTexture = swapChain.getCurrentTextureView();
	checkNullPointerError(nextTexture, "nextTexture");
  if(!nextTexture) {
    throw std::runtime_error("Failed to get next surface texture view");
    return;
  }

  // 创建 command encoder
  wgpu::CommandEncoderDescriptor encoderDesc = {};
  encoderDesc.label = "Command encoder";
  wgpu::CommandEncoder encoder = device.createCommandEncoder(encoderDesc);
	checkNullPointerError(encoder, "commandencoder");
	// LOG("Command encoder\n");

  // 创建 render pass 描述符
  wgpu::RenderPassDescriptor renderPassDesc = {};

  // 渲染管道描述符的 attachment 描述了通道的目标纹理
  wgpu::RenderPassColorAttachment colorAttachment = {};
  colorAttachment.view = nextTexture;
  colorAttachment.resolveTarget = nullptr;
  colorAttachment.loadOp = wgpu::LoadOp::Clear;
  colorAttachment.storeOp = wgpu::StoreOp::Store;
  colorAttachment.clearValue = wgpu::Color{ 0.05, 0.05, 0.05, 1.0 };
#ifndef WEBGPU_BACKEND_WGPU
	colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU
  renderPassDesc.colorAttachmentCount = 1;
  renderPassDesc.colorAttachments = &colorAttachment;
	// LOG("RenderPassDescriptor\n");

	// 创建深度缓冲区
	wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
	depthStencilAttachment.view = depthTextureView;
	depthStencilAttachment.depthClearValue = 1.0f;
	depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
	depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
	depthStencilAttachment.depthReadOnly = false;
	depthStencilAttachment.stencilClearValue = 0;
#ifdef WEBGPU_BACKEND_WGPU
	depthStencilAttachment.stencilLoadOp = LoadOp::Clear;
	depthStencilAttachment.stencilStoreOp = StoreOp::Store;
#else
	depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Undefined;
	depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Undefined;
#endif
	depthStencilAttachment.stencilReadOnly = true;
	// LOG("DepthStencilAttachment");

  renderPassDesc.depthStencilAttachment = &depthStencilAttachment;


	renderPassDesc.timestampWrites = 0;
  renderPassDesc.timestampWrites = nullptr;

  // 创建 render pass
  wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
	checkNullPointerError(renderPass, "renderPass");
  
	// 选择使用的 pipeline
	renderPass.setPipeline(pipeline);
	// 设置 vertex buffer
	renderPass.setVertexBuffer(0, vertexBuffer, 0, vertexData.size() * sizeof(VertexAttributes)); 
	// 设置 binding group
	renderPass.setBindGroup(0, bindGroup, 0, nullptr);

	// Draw 1 instance of a 3-vertices shape
	renderPass.draw(static_cast<int>(vertexData.size()), 1, 0, 0);

	renderPass.end();
	renderPass.release();

	nextTexture.release();

  // 执行 encoder 并且提交
  wgpu::CommandBufferDescriptor cmdBufferDesacriptor = {};
  // cmdBufferDesacriptor.nextInChain = nullptr;
  cmdBufferDesacriptor.label = "Command buffer Descriptor";

  wgpu::CommandBuffer command = encoder.finish(cmdBufferDesacriptor);
	checkNullPointerError(command, "command");
  encoder.release(); // <--  释放 encoder

	// LOG("Submitting command...\n");
	queue.submit(command);
	command.release();
	// LOG("Command submitted.\n");
	swapChain.present();
	// At the end of the frame

// #ifndef __EMSCRIPTEN__
// 	surface.present();
// #endif

#if defined(WEBGPU_BACKEND_DAWN)
	device.tick();
#elif defined(WEBGPU_BACKEND_WGPU)
	device.poll(false);
#endif

	// LOG("Main loop end\n");
}


bool Application::IsRunning() {
	return !glfwWindowShouldClose(wgpuGLFWWindow.window);
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
