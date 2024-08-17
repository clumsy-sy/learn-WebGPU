#include "application.h"

namespace webgpu {

bool Application::Initialize() {
	// glfw 初始化
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // <-- extra info for glfwCreateWindow
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(800, 600, "Learn WebGPU", nullptr, nullptr);
	
	// 创建实例
	WGPUInstance instance = wgpuCreateInstance(nullptr);
	
	// 获取适配器
	std::cout << "Requesting adapter...\n";
	surface = glfwGetWGPUSurface(instance, window);
	
	WGPURequestAdapterOptions adapterOpts = {};
	adapterOpts.nextInChain = nullptr;
	adapterOpts.compatibleSurface = surface;
	
	WGPUAdapter adapter = requestAdapterSync(instance, &adapterOpts);
	std::cout << "Got adapter: " << adapter << '\n';
	
	// 不在需要 instance，释放
	wgpuInstanceRelease(instance);
	
	// 获取 device
	std::cout << "Requesting device..." << '\n';
	WGPUDeviceDescriptor deviceDesc = {};
	deviceDesc.nextInChain = nullptr;
	deviceDesc.label = "Physical Device"; // anything works here, that's your call
	deviceDesc.requiredFeatureCount = 0; // we do not require any specific feature
	deviceDesc.requiredLimits = nullptr; // we do not require any specific limit
	deviceDesc.defaultQueue.nextInChain = nullptr;
	deviceDesc.defaultQueue.label = "The default queue";
	// A function that is invoked whenever the device stops being available.
	deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */) {
		std::cout << "Device lost: reason " << reason;
		if (message) std::cout << " (" << message << ")";
		std::cout << '\n';
	};
	device = requestDeviceSync(adapter, &deviceDesc);
	std::cout << "Got device: " << device << '\n';
	
	// We no longer need to access the adapter
	wgpuAdapterRelease(adapter);
	
	// Device error callback
	auto onDeviceError = [](WGPUErrorType type, char const* message, void* /* pUserData */) {
		std::cout << "Uncaptured device error: type " << type;
		if (message) std::cout << " (" << message << ")";
		std::cout << '\n';
	};
	wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr /* pUserData */);
	
  // 获取 队列
	queue = wgpuDeviceGetQueue(device);
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

	// Also move here the tick/poll but NOT the emscripten sleep
// #if defined(WEBGPU_BACKEND_DAWN)
	wgpuDeviceTick(device);
// #elif defined(WEBGPU_BACKEND_WGPU)
// 	wgpuDevicePoll(device, false, nullptr);
// #endif
}


bool Application::IsRunning() {
	return !glfwWindowShouldClose(window);
}

}
