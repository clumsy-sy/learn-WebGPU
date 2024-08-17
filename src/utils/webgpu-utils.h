#pragma once

#include <webgpu/webgpu.h>

namespace webgpu {

// 相当于   const adapter = await navigator.gpu.requestAdapter(options);
auto requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const * options) -> WGPUAdapter;


// 相当于  const device = await adapter.requestDevice(descriptor);
auto requestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor) -> WGPUDevice;


// 通过适配器对象检查硬件功能
void inspectAdapter(WGPUAdapter adapter);


// 显示一个设备的基本信息
void inspectDevice(WGPUDevice device);

}