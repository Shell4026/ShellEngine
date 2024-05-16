﻿#pragma once

#include "Render/Export.h"
#include "VulkanImpl/VulkanConfig.h"
#include "Shader.h"
#include "VulkanImpl/VulkanPipeline.h"

#include "Core/NonCopyable.h"

#include <vector>
#include <memory>

namespace sh::render
{
	class VulkanShader : public Shader
	{
		SCLASS(VulkanShader)
	private:
		VkDevice device;

		VkShaderModule vertShader;
		VkShaderModule fragShader;
	public:
		SH_RENDER_API VulkanShader(int id, VkDevice device);
		SH_RENDER_API VulkanShader(VulkanShader&& other) noexcept;
		SH_RENDER_API ~VulkanShader();
		
		SH_RENDER_API void Clean() override;

		SH_RENDER_API void SetVertexShader(VkShaderModule shader);
		SH_RENDER_API void SetFragmentShader(VkShaderModule shader);

		SH_RENDER_API auto GetVertexShader() const -> const VkShaderModule;
		SH_RENDER_API auto GetFragmentShader() const -> const VkShaderModule;
	};
}