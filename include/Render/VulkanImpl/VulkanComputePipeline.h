#pragma once
#include "../Export.h"
#include "../ShaderAST.h"
#include "VulkanConfig.h"

#include "Core/NonCopyable.h"

#include <vector>
#include <map>
#include <optional>
#include <array>
namespace sh::render
{
	class ComputeShader;
}
namespace sh::render::vk
{
	class VulkanContext;

	/// @brief 컴퓨트 셰이더 파이프라인.
	/// VkShaderModule, 디스크립터셋 레이아웃, 파이프라인 레이아웃, VkPipeline을 모두 보관한다.
	class VulkanComputePipeline : public core::INonCopyable
	{
	public:
		struct SetLayout
		{
			uint32_t set = 0;
			VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
			std::map<uint32_t, VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
		};
	public:
		SH_RENDER_API VulkanComputePipeline(const VulkanContext& context, const ComputeShader& shader);
		SH_RENDER_API VulkanComputePipeline(VulkanComputePipeline&& other) noexcept;
		SH_RENDER_API ~VulkanComputePipeline();

		/// @brief 셰이더 모듈, 디스크립터/파이프라인 레이아웃, VkPipeline까지 한번에 생성한다.
		SH_RENDER_API auto Build() -> VkResult;
		SH_RENDER_API void Clean();

		SH_RENDER_API auto GetPipeline() const -> VkPipeline { return pipeline; }
		SH_RENDER_API auto GetPipelineLayout() const -> VkPipelineLayout { return pipelineLayout; }
		SH_RENDER_API auto GetShaderModule() const -> VkShaderModule { return shaderModule; }

		SH_RENDER_API auto GetSetLayouts() const -> const std::vector<SetLayout>& { return setlayouts; }
		SH_RENDER_API auto GetDescriptorSetLayout(uint32_t set) const -> VkDescriptorSetLayout;
		SH_RENDER_API auto GetDescriptorBinding(uint32_t set, uint32_t binding) const -> std::optional<VkDescriptorSetLayoutBinding>;

		/// @brief Numthreads (local_size) 반환.
		SH_RENDER_API auto GetGroupSize() const -> std::array<uint32_t, 3>;
	private:
		auto CreateShaderModule() -> VkResult;
		void AddDescriptorBinding(uint32_t set, uint32_t binding, VkDescriptorType type);
		auto CreateDescriptorLayout() -> VkResult;
		auto CreatePipelineLayout() -> VkResult;
		auto CreatePipeline() -> VkResult;

		void CleanDescriptors();
	private:
		const VulkanContext& context;
		const ComputeShader* shader;

		VkShaderModule shaderModule = VK_NULL_HANDLE;
		std::vector<SetLayout> setlayouts;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;
	};
}//namespace
