﻿#pragma once
#include "Export.h"
#include "VulkanConfig.h"

#include <vector>
#include <string_view>

namespace sh::render::vk
{
	class VulkanLayer
	{
	public:
		struct LayerProperties 
		{
			VkLayerProperties properties;
			std::vector<VkExtensionProperties> extensions;

			SH_RENDER_API LayerProperties() = default;
			SH_RENDER_API LayerProperties(const LayerProperties& other);
			SH_RENDER_API LayerProperties(LayerProperties& other);
			SH_RENDER_API LayerProperties(LayerProperties&& other) noexcept;
		};
	private:
		std::vector<LayerProperties> layers;
		std::vector<VkExtensionProperties> vulkanExtensions;

		std::vector<LayerProperties> gpuLayers;
		std::vector<VkExtensionProperties> gpuExtensions;
	private:
		SH_RENDER_API auto QueryLayerExtensions(LayerProperties& layerProp, VkPhysicalDevice gpu = nullptr)->VkResult;
		SH_RENDER_API auto QueryVulkanExtensions()->VkResult;
		SH_RENDER_API auto QueryGPUExtensions(VkPhysicalDevice gpu)->VkResult;
	public:
		SH_RENDER_API VulkanLayer();
		SH_RENDER_API ~VulkanLayer();

		/// @brief 레이어와 확장을 탐색한다.
		/// @param gpu nullptr일 시 인스턴스의 레이어와 확장을 탐색한다.
		SH_RENDER_API void Query(VkPhysicalDevice gpu = nullptr);

		SH_RENDER_API bool FindLayer(std::string_view layerName, VkPhysicalDevice gpu = nullptr);
		SH_RENDER_API bool FindExtension(std::string_view extensionName);
		SH_RENDER_API bool FindGPUExtension(VkPhysicalDevice gpu, std::string_view extensionName);

		SH_RENDER_API auto GetLayerProperties() const->const std::vector<LayerProperties>&;
		SH_RENDER_API auto GetVulkanExtensions() const->const std::vector<VkExtensionProperties>&;
		SH_RENDER_API auto GetGPULayerProperties() const->const std::vector<LayerProperties>&;
		SH_RENDER_API auto GetGPUExtensions() const->const std::vector<VkExtensionProperties>&;
	};
}