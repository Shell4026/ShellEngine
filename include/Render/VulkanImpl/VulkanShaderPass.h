#pragma once

#include "../Export.h"
#include "../ShaderPass.h"
#include "VulkanConfig.h"
#include "VulkanPipeline.h"

#include "Core/SContainer.hpp"
#include "Core/NonCopyable.h"

#include <vector>
#include <memory>

namespace sh::render::vk
{
	class VulkanContext;
	class VulkanShaderPass : public ShaderPass
	{
		SCLASS(VulkanShaderPass)
	private:
		const VulkanContext& context;

		VkShaderModule vertShader;
		VkShaderModule fragShader;

		std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorBindings; // set, binding
		std::vector<VkDescriptorSetLayout> descriptorSetLayout;

		VkPipelineLayout pipelineLayout;

		bool bUseMatrixModel = false;
	private:
		void AddDescriptorBinding(uint32_t set, uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stage);
		auto CreateDescriptorLayout() -> VkResult;
		auto CreatePipelineLayout() -> VkResult;

		void CleanDescriptors();
	public:
		struct ShaderModules
		{
			VkShaderModule vert;
			VkShaderModule frag;
		};
		struct DescryptorSetLayoutInfo
		{
			VkDescriptorSetLayout layout;
			bool bDynamic;
		};
	public:
		SH_RENDER_API VulkanShaderPass(const VulkanContext& context, const ShaderModules& shaderModules, const ShaderAST::PassNode& passNode);
		SH_RENDER_API VulkanShaderPass(VulkanShaderPass&& other) noexcept;
		SH_RENDER_API ~VulkanShaderPass();
		
		SH_RENDER_API void Clear() override;

		SH_RENDER_API auto GetVertexShader() const -> const VkShaderModule;
		SH_RENDER_API auto GetFragmentShader() const -> const VkShaderModule;

		SH_RENDER_API void Build();

		/// @brief 디스크립터셋 레이아웃을 반환 하는 함수
		/// @param set 세트 번호
		/// @return 디스크립터셋 레이아웃 포인터
		SH_RENDER_API auto GetDescriptorSetLayout(uint32_t set) const -> DescryptorSetLayoutInfo;
		SH_RENDER_API auto GetSetCount() const -> uint32_t;
		SH_RENDER_API auto GetPipelineLayout() const -> VkPipelineLayout;
	};
}