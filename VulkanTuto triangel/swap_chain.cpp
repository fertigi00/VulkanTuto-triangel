#include "swap_chain.h"
//vk::raii::ShaderModule shaderModule = createShaderModule(readFile("shaders/slang.spv")); // need to creat it 

	//vk::PipelineShaderStageCreateInfo vertShaderStageInfo{ .stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule, .pName = "vertMain" };
	//vk::PipelineShaderStageCreateInfo fragShaderStageInfo{ .stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "fragMain" };
	//vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };


//[[nodiscard]] vk::raii::ShaderModule Renderer::createShaderModule(const std::vector<char>& code) const
//{
//	vk::ShaderModuleCreateInfo createInfo{ .codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(code.data()) };
//	vk::raii::ShaderModule     shaderModule{ device, createInfo };
//
//	return shaderModule;
//}
//
//static std::vector<char> readFile(const std::string& filename)
//{
//	std::ifstream file(filename, std::ios::ate | std::ios::binary);
//	if (!file.is_open())
//	{
//		throw std::runtime_error("failed to open file!");
//	}
//	std::vector<char> buffer(file.tellg());
//	file.seekg(0, std::ios::beg);
//	file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
//	file.close();
//	return buffer;
//}