#pragma once
#include "../Export.h"
#include "../ShaderPass.h"

#include <vector>
#include <memory>
#include <map>
#include <optional>
namespace sh::render::vk
{
	class VulkanContext;
	class VulkanShaderPass : public ShaderPass
	{
		SCLASS(VulkanShaderPass)
	public:
		struct ShaderModules
		{
			VkShaderModule vert;
			VkShaderModule frag;
		};
		struct SetLayout
		{
			uint32_t set = 0;
			VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
			std::map<uint32_t, VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
		};
	public:
		SH_RENDER_API VulkanShaderPass(const VulkanContext& context, const ShaderModules& shaderModules, const ShaderAST::PassNode& passNode);
		SH_RENDER_API VulkanShaderPass(VulkanShaderPass&& other) noexcept;
		SH_RENDER_API ~VulkanShaderPass();
		
		SH_RENDER_API void Clear() override;

		SH_RENDER_API void Build();

		SH_RENDER_API auto GetVertexShader() const -> const VkShaderModule { return vertShader; }
		SH_RENDER_API auto GetFragmentShader() const -> const VkShaderModule { return fragShader; }
		SH_RENDER_API auto GetPipelineLayout() const -> VkPipelineLayout { return pipelineLayout; }
		/// @brief 디스크립터셋 레이아웃을 반환 하는 함수
		/// @param set 세트 번호
		/// @return 디스크립터셋 레이아웃 포인터
		SH_RENDER_API auto GetDescriptorSetLayout(uint32_t set) const -> VkDescriptorSetLayout;
		SH_RENDER_API auto GetSetCount() const -> std::size_t { return setlayouts.size(); }
		SH_RENDER_API auto GetSetLayout(uint32_t set) const -> std::optional<SetLayout>;
		SH_RENDER_API auto GetSetLayouts() const -> const std::vector<SetLayout>& { return setlayouts; }
		SH_RENDER_API auto GetSpecializationMapEntry() const -> const std::vector<VkSpecializationMapEntry>& { return specializationEntry; }
		SH_RENDER_API auto GetDescriptorBinding(uint32_t set, uint32_t binding) const -> std::optional<VkDescriptorSetLayoutBinding>;
	private:
		void AddDescriptorBinding(uint32_t set, uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stage);
		auto CreateDescriptorLayout() -> VkResult;
		auto CreatePipelineLayout() -> VkResult;

		void CleanDescriptors();
	private:
		const VulkanContext& context;

		VkShaderModule vertShader;
		VkShaderModule fragShader;

		std::vector<SetLayout> setlayouts;

		VkPipelineLayout pipelineLayout;

		std::vector<VkSpecializationMapEntry> specializationEntry;

		bool bUseMatrixModel = false;
	};
}//namespace