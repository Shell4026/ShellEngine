#pragma once

#include "../Export.h"
#include "../Shader.h"
#include "VulkanConfig.h"
#include "VulkanPipeline.h"

#include "Core/SContainer.hpp"
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

		core::SVector<VkDescriptorSetLayoutBinding> localDescriptorBindings;
		core::SVector<VkDescriptorSetLayoutBinding> descriptorBindings;
		core::SVector<VkDescriptorSetLayout> descriptorSetLayout;

		VkPipelineLayout pipelineLayout;
	private:
		void AddDescriptorBinding(uint32_t set, uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stage);
		auto CreateDescriptorLayout() -> VkResult;
		auto CreatePipelineLayout() -> VkResult;

		void CleanDescriptors();
	public:
		SH_RENDER_API VulkanShader(int id, VkDevice device);
		SH_RENDER_API VulkanShader(VulkanShader&& other) noexcept;
		SH_RENDER_API ~VulkanShader();
		
		SH_RENDER_API void Clean() override;

		SH_RENDER_API void SetVertexShader(VkShaderModule shader);
		SH_RENDER_API void SetFragmentShader(VkShaderModule shader);

		SH_RENDER_API auto GetVertexShader() const -> const VkShaderModule;
		SH_RENDER_API auto GetFragmentShader() const -> const VkShaderModule;

		SH_RENDER_API void Build();

		/// @brief 디스크립터셋 레이아웃을 반환 하는 함수
		/// @param set 세트 번호
		/// @return 디스크립터셋 레이아웃 포인터
		SH_RENDER_API auto GetDescriptorSetLayout(uint32_t set) const -> VkDescriptorSetLayout;
		SH_RENDER_API auto GetPipelineLayout() const -> VkPipelineLayout;
	};
}