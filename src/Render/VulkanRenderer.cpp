#include "VulkanRenderer.h"

#include <vulkan/vulkan.h>
#include <fmt/core.h>
#include <vector>

namespace sh::render {
	VulkanRenderer::VulkanRenderer()
	{

	}
	VulkanRenderer::~VulkanRenderer()
	{
	}

	void VulkanRenderer::Init()
	{
		std::vector<VkLayerProperties> layerProperties;
		uint32_t instanceLayerCount = 0;

		VkResult result;
		do
		{
			result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, NULL);

			fmt::print("Vulkan 레이어 수 : {}\n", instanceLayerCount);
			layerProperties.resize(instanceLayerCount);
			
			result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, layerProperties.data());
		} while (result == VK_INCOMPLETE);
		
		fmt::print("Instanced Layers:\n");
		for (auto prop : layerProperties)
		{
			fmt::print("Layer: {}\n", prop.layerName);
			fmt::print("Description: {}\n", prop.description);
		}
	}
}//namespace

