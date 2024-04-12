#include "VulkanLayer.h"

#include <cassert>
namespace sh::render
{
	auto VulkanLayer::GetLayerExtensions(LayerProperties& layerProp, VkPhysicalDevice gpu) -> VkResult
	{
		uint32_t extensionCount = 0;
		VkResult result;

		char* layerName = layerProp.properties.layerName;

		if(gpu)
			result = vkEnumerateDeviceExtensionProperties(gpu, layerName, &extensionCount, nullptr);
		else
			result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr);
		assert(result == VkResult::VK_SUCCESS);

		layerProp.extensions.resize(extensionCount);

		if(gpu)
			result = vkEnumerateDeviceExtensionProperties(gpu, layerName, &extensionCount, layerProp.extensions.data());
		else
			result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, layerProp.extensions.data());

		return result;
	}
	auto VulkanLayer::GetGPUExtensions(VkPhysicalDevice gpu) -> VkResult
	{
		uint32_t extensionCount = 0;
		VkResult result;
		result = vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr);
		assert(result == VkResult::VK_SUCCESS);
		gpuExtensions.resize(extensionCount);

		result = vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, gpuExtensions.data());
		return result;
	}

	void VulkanLayer::Query(VkPhysicalDevice gpu)
	{
		std::vector<VkLayerProperties> layerProperties;
		uint32_t instanceLayerCount = 0;

		VkResult result;
		if (gpu)
			result = vkEnumerateDeviceLayerProperties(gpu, &instanceLayerCount, nullptr);
		else
			result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
		assert(result == VkResult::VK_SUCCESS);

		layerProperties.resize(instanceLayerCount);

		if (gpu)
			result = vkEnumerateDeviceLayerProperties(gpu, &instanceLayerCount, layerProperties.data());
		else
			result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, layerProperties.data());
		assert(result == VkResult::VK_SUCCESS);

		for (auto& prop : layerProperties)
		{
			LayerProperties layerProp{};
			layerProp.properties = prop;

			result = GetLayerExtensions(layerProp, gpu);
			if (result != VkResult::VK_SUCCESS)
				continue;

			if (gpu)
				gpuLayers.push_back(std::move(layerProp));
			else
				layers.push_back(std::move(layerProp));
			assert(result == VkResult::VK_SUCCESS);
		}

		if (gpu)
			result = GetGPUExtensions(gpu);
		assert(result == VkResult::VK_SUCCESS);
	}
	VulkanLayer::LayerProperties::LayerProperties(const LayerProperties& other)
	{
		properties = other.properties;
		extensions = other.extensions;
	}
	VulkanLayer::LayerProperties::LayerProperties(LayerProperties& other)
	{
		properties = other.properties;
		extensions = other.extensions;
	}
	VulkanLayer::LayerProperties::LayerProperties(LayerProperties&& other) noexcept
	{
		properties = other.properties;
		extensions = std::move(other.extensions);
	}

	bool VulkanLayer::FindLayer(std::string_view layerName, VkPhysicalDevice gpu)
	{
		auto& layersVector = gpu ? gpuLayers : layers;
		for (auto& i : layers)
		{
			if (layerName == i.properties.layerName)
				return true;
		}
		return false;
	}

	bool VulkanLayer::FindGPUExtension(VkPhysicalDevice gpu, std::string_view extensionName)
	{
		for (auto& i : gpuExtensions)
		{
			if (extensionName == i.extensionName)
				return true;
		}
		return false;
	}

	auto VulkanLayer::GetLayerProperties() const -> const std::vector<LayerProperties>&
	{
		return layers;
	}
	auto VulkanLayer::GetGPULayerProperties() const -> const std::vector<LayerProperties>&
	{
		return gpuLayers;
	}
	auto VulkanLayer::GetGPUExtensions() const -> const std::vector<VkExtensionProperties>&
	{
		return gpuExtensions;
	}
}