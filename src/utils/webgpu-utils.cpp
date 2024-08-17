#include "webgpu-utils.h"

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__

#include <iostream>
#include <vector>
#include <cassert>


namespace webgpu {

/**
 * @brief 同步请求 WebGPU Adapter
 *
 * 此函数同步请求一个 WebGPU Adapter。它通过异步请求并等待结果的方式实现同步行为。
 * 
 * @param instance WebGPU 实例。
 * @param options 请求 Adapter 的选项。
 * @return 返回请求到的 WebGPU Adapter。
 */
auto requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const * options) -> WGPUAdapter {
	// 一个简单的结构体，用于保存与 onAdapterRequestEnded 回调共享的本地信息。
	struct UserData {
		WGPUAdapter adapter = nullptr;
		bool requestEnded = false;
	};
	UserData userData;

	// 当请求返回时由 wgpuInstanceRequestAdapter 调用的回调函数。
	// 这是一个 C++ 的 lambda 函数，但也可以是全局作用域中的任何函数。
	// 它必须是非捕获的（括号 [] 是空的），以便像常规的 C 函数指针那样工作，
	// 这正是 wgpuInstanceRequestAdapter 所期望的（因为 WebGPU 是一个 C API）。
	// 解决方案是通过 pUserData 指针传递我们想要捕获的内容，
	// 这个指针作为 wgpuInstanceRequestAdapter 的最后一个参数提供，并被回调函数接收为其最后一个参数。
	auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const * message, void * pUserData) {
		UserData& userData = *reinterpret_cast<UserData*>(pUserData);
		if (status == WGPURequestAdapterStatus_Success) {
			userData.adapter = adapter;
		} else {
			std::cout << "无法获取 WebGPU adapter: " << message << '\n';
		}
		userData.requestEnded = true;
	};

	// 调用 WebGPU 请求 adapter 的过程
	wgpuInstanceRequestAdapter(
		instance /* 相当于 navigator.gpu */,
		options,
		onAdapterRequestEnded,
		(void*)&userData
	);

	// 我们等待直到 userData.requestEnded 变为 true
#ifdef __EMSCRIPTEN__
	while (!userData.requestEnded) {
		emscripten_sleep(100);
	}
#endif // __EMSCRIPTEN__

	assert(userData.requestEnded);

	return userData.adapter;
}


/**
 * @brief 检查并打印指定适配器的信息
 *
 * 此函数用于检查并打印给定 WebGPU 适配器的限制、特性以及属性信息。
 * 
 * @param adapter 需要检查的 WebGPU 适配器。
 */
void inspectAdapter(WGPUAdapter adapter) {
#ifndef __EMSCRIPTEN__
	WGPUSupportedLimits supportedLimits = {};
	supportedLimits.nextInChain = nullptr;

	bool success = wgpuAdapterGetLimits(adapter, &supportedLimits) == WGPUStatus_Success;

	if (success) {
		std::cout << "Adapter limits:\n";
		std::cout << " - maxTextureDimension1D: " << supportedLimits.limits.maxTextureDimension1D << '\n';
		std::cout << " - maxTextureDimension2D: " << supportedLimits.limits.maxTextureDimension2D << '\n';
		std::cout << " - maxTextureDimension3D: " << supportedLimits.limits.maxTextureDimension3D << '\n';
		std::cout << " - maxTextureArrayLayers: " << supportedLimits.limits.maxTextureArrayLayers << '\n';
	}
#endif // NOT __EMSCRIPTEN__
	// 创建一个动态大小的向量来存储适配器特性
	std::vector<WGPUFeatureName> features;

	// 第一次调用以获取特性数量
	size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, nullptr);

	// 分配内存以存储特性列表
	features.resize(featureCount);

	// 第二次调用以填充特性列表
	wgpuAdapterEnumerateFeatures(adapter, features.data());

	// 打印适配器特性
	std::cout << "Adapter features:" << '\n';
	std::cout << std::hex; // 将整数以十六进制形式输出，便于与 webgpu.h 中的常量比较
	for (auto f : features) {
			std::cout << " - 0x" << f << '\n';
	}
	std::cout << std::dec; // 恢复十进制数字输出

	WGPUAdapterProperties properties = {};
	properties.nextInChain = nullptr;
	wgpuAdapterGetProperties(adapter, &properties);
	std::cout << "Adapter properties:" << '\n';
	std::cout << " - vendorID: " << properties.vendorID << '\n';
	if (properties.vendorName) {
		std::cout << " - vendorName: " << properties.vendorName << '\n';
	}
	if (properties.architecture) {
		std::cout << " - architecture: " << properties.architecture << '\n';
	}
	std::cout << " - deviceID: " << properties.deviceID << '\n';
	if (properties.name) {
		std::cout << " - name: " << properties.name << '\n';
	}
	if (properties.driverDescription) {
		std::cout << " - driverDescription: " << properties.driverDescription << '\n';
	}
	std::cout << std::hex;
	std::cout << " - adapterType: 0x" << properties.adapterType << '\n';
	std::cout << " - backendType: 0x" << properties.backendType << '\n';
	std::cout << std::dec; // 恢复十进制数字输出
}

/**
 * @brief 请求并同步获取WebGPU设备
 *
 * 此函数用于请求WebGPU设备，并等待直到设备请求完成。如果在Emscripten环境下，
 * 则会通过循环等待直到请求结束。
 *
 * @param adapter 适配器对象，用于请求设备。
 * @param descriptor 设备描述符，定义设备的配置。
 * @return 返回请求到的WebGPU设备。
 */
WGPUDevice requestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor) {
	struct UserData {
		WGPUDevice device = nullptr;
		bool requestEnded = false;
	};
	UserData userData;

	auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status, WGPUDevice device, char const * message, void * pUserData) {
		UserData& userData = *reinterpret_cast<UserData*>(pUserData);
		if (status == WGPURequestDeviceStatus_Success) {
			userData.device = device;
		} else {
			std::cout << "Could not get WebGPU device: " << message << '\n';
		}
		userData.requestEnded = true;
	};

	wgpuAdapterRequestDevice(
		adapter,
		descriptor,
		onDeviceRequestEnded,
		(void*)&userData
	);

#ifdef __EMSCRIPTEN__
	while (!userData.requestEnded) {
		emscripten_sleep(100);
	}
#endif // __EMSCRIPTEN__

	assert(userData.requestEnded);

	return userData.device;
}

/**
 * @brief 检查并打印WebGPU设备的功能和限制
 *
 * 此函数首先枚举设备支持的所有功能，并打印它们的十六进制值。
 * 然后尝试获取设备的限制信息，并打印出来。
 *
 * @param device 需要检查的WebGPU设备。
 */
void inspectDevice(WGPUDevice device) {
	std::vector<WGPUFeatureName> features;
	size_t featureCount = wgpuDeviceEnumerateFeatures(device, nullptr);
	features.resize(featureCount);
	wgpuDeviceEnumerateFeatures(device, features.data());

	std::cout << "Device features:" << '\n';
	std::cout << std::hex;
	for (auto f : features) {
		std::cout << " - 0x" << f << '\n';
	}
	std::cout << std::dec;

	WGPUSupportedLimits limits = {};
	limits.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_DAWN
	bool success = wgpuDeviceGetLimits(device, &limits) == WGPUStatus_Success;
#else
	bool success = wgpuDeviceGetLimits(device, &limits);
#endif
	
	if (success) {
		std::cout << "Device limits:" << '\n';
		std::cout << " - maxTextureDimension1D: " << limits.limits.maxTextureDimension1D << '\n';
		std::cout << " - maxTextureDimension2D: " << limits.limits.maxTextureDimension2D << '\n';
		std::cout << " - maxTextureDimension3D: " << limits.limits.maxTextureDimension3D << '\n';
		std::cout << " - maxTextureArrayLayers: " << limits.limits.maxTextureArrayLayers << '\n';
		std::cout << " - maxBindGroups: " << limits.limits.maxBindGroups << '\n';
		std::cout << " - maxDynamicUniformBuffersPerPipelineLayout: " << limits.limits.maxDynamicUniformBuffersPerPipelineLayout << '\n';
		std::cout << " - maxDynamicStorageBuffersPerPipelineLayout: " << limits.limits.maxDynamicStorageBuffersPerPipelineLayout << '\n';
		std::cout << " - maxSampledTexturesPerShaderStage: " << limits.limits.maxSampledTexturesPerShaderStage << '\n';
		std::cout << " - maxSamplersPerShaderStage: " << limits.limits.maxSamplersPerShaderStage << '\n';
		std::cout << " - maxStorageBuffersPerShaderStage: " << limits.limits.maxStorageBuffersPerShaderStage << '\n';
		std::cout << " - maxStorageTexturesPerShaderStage: " << limits.limits.maxStorageTexturesPerShaderStage << '\n';
		std::cout << " - maxUniformBuffersPerShaderStage: " << limits.limits.maxUniformBuffersPerShaderStage << '\n';
		std::cout << " - maxUniformBufferBindingSize: " << limits.limits.maxUniformBufferBindingSize << '\n';
		std::cout << " - maxStorageBufferBindingSize: " << limits.limits.maxStorageBufferBindingSize << '\n';
		std::cout << " - minUniformBufferOffsetAlignment: " << limits.limits.minUniformBufferOffsetAlignment << '\n';
		std::cout << " - minStorageBufferOffsetAlignment: " << limits.limits.minStorageBufferOffsetAlignment << '\n';
		std::cout << " - maxVertexBuffers: " << limits.limits.maxVertexBuffers << '\n';
		std::cout << " - maxVertexAttributes: " << limits.limits.maxVertexAttributes << '\n';
		std::cout << " - maxVertexBufferArrayStride: " << limits.limits.maxVertexBufferArrayStride << '\n';
		std::cout << " - maxInterStageShaderComponents: " << limits.limits.maxInterStageShaderComponents << '\n';
		std::cout << " - maxComputeWorkgroupStorageSize: " << limits.limits.maxComputeWorkgroupStorageSize << '\n';
		std::cout << " - maxComputeInvocationsPerWorkgroup: " << limits.limits.maxComputeInvocationsPerWorkgroup << '\n';
		std::cout << " - maxComputeWorkgroupSizeX: " << limits.limits.maxComputeWorkgroupSizeX << '\n';
		std::cout << " - maxComputeWorkgroupSizeY: " << limits.limits.maxComputeWorkgroupSizeY << '\n';
		std::cout << " - maxComputeWorkgroupSizeZ: " << limits.limits.maxComputeWorkgroupSizeZ << '\n';
		std::cout << " - maxComputeWorkgroupsPerDimension: " << limits.limits.maxComputeWorkgroupsPerDimension << '\n';
	}
}

}
