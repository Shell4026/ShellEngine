#include "VulkanImpl/VulkanLayer.h"

#include <cassert>
namespace sh::render::vk
{
	VulkanLayer::VulkanLayer()
	{
	}

	VulkanLayer::~VulkanLayer()
	{
	}

	auto VulkanLayer::QueryLayerExtensions(LayerProperties& layerProp, VkPhysicalDevice gpu) -> VkResult
	{
		uint32_t extensionCount = 0;
		VkResult result;

		char* layerName = layerProp.properties.layerName;

		if (gpu == nullptr) // instance 확장 쿼리
		{
			result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr);
			assert(result == VkResult::VK_SUCCESS);
			layerProp.extensions.resize(extensionCount);
			result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, layerProp.extensions.data());
			assert(result == VkResult::VK_SUCCESS);
		}
		else // GPU 확장 쿼리
		{
			result = vkEnumerateDeviceExtensionProperties(gpu, layerName, &extensionCount, nullptr);
			assert(result == VkResult::VK_SUCCESS);
			layerProp.extensions.resize(extensionCount);
			result = vkEnumerateDeviceExtensionProperties(gpu, layerName, &extensionCount, layerProp.extensions.data());
			assert(result == VkResult::VK_SUCCESS);
		}
		return result;
	}

	auto VulkanLayer::QueryVulkanExtensions() -> VkResult
	{
		uint32_t extensionCount = 0;
		VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		assert(result == VkResult::VK_SUCCESS);

		vulkanExtensions.resize(extensionCount);

		result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vulkanExtensions.data());
		assert(result == VkResult::VK_SUCCESS);

		return result;
	}

	auto VulkanLayer::QueryGPUExtensions(VkPhysicalDevice gpu) -> VkResult
	{
		gpuExtensions.clear();

		uint32_t extensionCount = 0;
		VkResult result;
		result = vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr);
		assert(result == VkResult::VK_SUCCESS);
		gpuExtensions.resize(extensionCount);

		result = vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, gpuExtensions.data());
		assert(result == VkResult::VK_SUCCESS);
		return result;
	}

	void VulkanLayer::Query(VkPhysicalDevice gpu)
	{
		std::vector<VkLayerProperties> layerProperties;
		uint32_t layerCount = 0;
		VkResult result;

		if (gpu == nullptr) // instance 레이어 쿼리
		{
			layers.clear();

			result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
			assert(result == VkResult::VK_SUCCESS);

			layerProperties.resize(layerCount);
			layers.reserve(layerCount);

			result = vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
			assert(result == VkResult::VK_SUCCESS);

			for (auto& prop : layerProperties)
			{
				LayerProperties layerProp{};
				layerProp.properties = prop;

				result = QueryLayerExtensions(layerProp, gpu);
				if (result != VkResult::VK_SUCCESS)
					continue;

				layers.push_back(std::move(layerProp));
			}

			result = QueryVulkanExtensions();
			assert(result == VkResult::VK_SUCCESS);
		}
		else // GPU 레이어 쿼리
		{
			gpuLayers.clear();

			result = vkEnumerateDeviceLayerProperties(gpu, &layerCount, nullptr);
			assert(result == VkResult::VK_SUCCESS);

			layerProperties.resize(layerCount);
			gpuLayers.reserve(layerCount);

			result = vkEnumerateDeviceLayerProperties(gpu, &layerCount, layerProperties.data());
			assert(result == VkResult::VK_SUCCESS);

			for (auto& prop : layerProperties)
			{
				LayerProperties layerProp{};
				layerProp.properties = prop;

				result = QueryLayerExtensions(layerProp, gpu);
				if (result != VkResult::VK_SUCCESS)
					continue;

				gpuLayers.push_back(std::move(layerProp));
			}

			result = QueryGPUExtensions(gpu);
			assert(result == VkResult::VK_SUCCESS);
		}
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
		for (auto& i : layersVector)
		{
			if (layerName == i.properties.layerName)
				return true;
		}
		return false;
	}

	bool VulkanLayer::FindExtension(std::string_view extensionName)
	{
		for (auto& i : vulkanExtensions)
		{
			if (extensionName == i.extensionName)
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
	auto VulkanLayer::GetVulkanExtensions() const -> const std::vector<VkExtensionProperties>&
	{
		return vulkanExtensions;
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