﻿#pragma once

#include "Export.h"

#include "VulkanConfig.h"

#include <vector>
#include <string_view>

namespace sh::render
{
	class VulkanLayer
	{
	public:
		struct LayerProperties {
			VkLayerProperties properties;
			std::vector<VkExtensionProperties> extensions;

			SH_RENDER_API LayerProperties() = default;
			SH_RENDER_API LayerProperties(const LayerProperties& other);
			SH_RENDER_API LayerProperties(LayerProperties& other);
			SH_RENDER_API LayerProperties(LayerProperties&& other) noexcept;
		};
		std::vector<VkExtensionProperties> gpuExtensions;
	private:
		std::vector<LayerProperties> layers;
		std::vector<LayerProperties> gpuLayers;
	private:
		SH_RENDER_API auto GetLayerExtensions(LayerProperties& layerProp, VkPhysicalDevice gpu = nullptr)->VkResult;
		SH_RENDER_API auto GetGPUExtensions(VkPhysicalDevice gpu)->VkResult;
	public:
		SH_RENDER_API void Query(VkPhysicalDevice gpu = nullptr);
		SH_RENDER_API bool FindLayer(std::string_view layerName, VkPhysicalDevice gpu = nullptr);
		SH_RENDER_API bool FindGPUExtension(VkPhysicalDevice gpu, std::string_view extensionName);
		SH_RENDER_API auto GetLayerProperties() const->const std::vector<LayerProperties>&;
		SH_RENDER_API auto GetGPULayerProperties() const->const std::vector<LayerProperties>&;
		SH_RENDER_API auto GetGPUExtensions() const->const std::vector<VkExtensionProperties>&;
	};
}