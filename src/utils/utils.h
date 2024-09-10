#pragma once

#include "global.h"

namespace webgpu {

wgpu::ShaderModule loadShaderModule(const std::filesystem::path& path, wgpu::Device &device);

}