#pragma once

#include "Renderer.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace sh::render {
	class VulkanRenderer : public Renderer {
	private:
		struct LayerProperties {
			VkLayerProperties properties;
			std::vector<VkExtensionProperties> extensions;
		};

		std::vector<LayerProperties> layers;
	private:
		auto GetInstanceLayerProperties()->VkResult;
		auto GetExtensionProperties(LayerProperties& layerProp, VkPhysicalDevice* gpu = nullptr)->VkResult;
	public:
		VulkanRenderer();
		~VulkanRenderer();

		bool Init() override;
	};
}//namespace
