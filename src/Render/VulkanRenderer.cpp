#include "VulkanRenderer.h"

#include <fmt/core.h>
#include "../Core/Util.h"

namespace sh::render {
	VulkanRenderer::VulkanRenderer()
	{

	}
	VulkanRenderer::~VulkanRenderer()
	{
	}

	auto VulkanRenderer::GetInstanceLayerProperties() -> VkResult
	{
		std::vector<VkLayerProperties> layerProperties;
		uint32_t instanceLayerCount = 0;

		VkResult result;
		do
		{
			result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);

			layerProperties.resize(instanceLayerCount);
			result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, layerProperties.data());
		} while (result == VkResult::VK_INCOMPLETE);

		for (auto& prop : layerProperties)
		{
			LayerProperties layerProp;
			layerProp.properties = prop;

			result = GetExtensionProperties(layerProp);
			if (result != VkResult::VK_SUCCESS)
				continue;

			layers.push_back(std::move(layerProp));
		}

		if (sh::core::Util::IsDebug())
		{
			for (auto& i : layers)
			{
				fmt::print("LayerName: {}\n", i.properties.layerName);
				for (auto& ext : i.extensions)
				{
					fmt::print("ExtensionName: {}\n", ext.extensionName);
				}
			}
		}
		return result;
	}

	auto VulkanRenderer::GetExtensionProperties(LayerProperties& layerProp, VkPhysicalDevice* gpu) -> VkResult
	{
		uint32_t extensionCount = 0;
		VkResult result;

		char* layerName = layerProp.properties.layerName;
		do
		{
			if (gpu)
				result = vkEnumerateDeviceExtensionProperties(*gpu, layerName, &extensionCount, nullptr);
			else
				result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr);

			if (result != VkResult::VK_SUCCESS || extensionCount == 0)
				continue;

			layerProp.extensions.resize(extensionCount);
			if(gpu)
				result = vkEnumerateDeviceExtensionProperties(*gpu, layerName, &extensionCount, layerProp.extensions.data());
			else
				result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, layerProp.extensions.data());
		} while (result == VkResult::VK_INCOMPLETE);

		return result;
	}

	bool VulkanRenderer::Init()
	{
		GetInstanceLayerProperties();
		return true;
	}
}//namespace

