#include "utils.h"
#include <stdexcept>


namespace webgpu {

wgpu::ShaderModule loadShaderModule(const std::filesystem::path& path, wgpu::Device &device){
  std::ifstream file(path);
	if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + path.string());
		// return nullptr;
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	std::string shaderSource(size, ' ');
	file.seekg(0);
	file.read(shaderSource.data(), size);

	wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
	shaderCodeDesc.chain.next = nullptr;
	shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
	shaderCodeDesc.code = shaderSource.c_str();
	wgpu::ShaderModuleDescriptor shaderDesc;
	shaderDesc.nextInChain = &shaderCodeDesc.chain;
#ifdef WEBGPU_BACKEND_WGPU
	shaderDesc.hintCount = 0;
	shaderDesc.hints = nullptr;
#endif

	return device.createShaderModule(shaderDesc);
}

}