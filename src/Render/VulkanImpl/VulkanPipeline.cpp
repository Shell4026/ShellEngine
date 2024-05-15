#include "VulkanPipeline.h"

#include "VulkanShader.h"
#include "VulkanSurface.h"

#include <cassert>
namespace sh::render::impl
{
	VulkanPipeline::VulkanPipeline(VkDevice device, VkRenderPass renderPass) :
		device(device), renderPass(renderPass),
		pipelineLayout(nullptr), pipeline(nullptr), shader(nullptr),
		viewportX(0), viewportY(0)
	{
	}

	VulkanPipeline::VulkanPipeline(VulkanPipeline&& other) noexcept :
		pipelineLayout(other.pipelineLayout), pipeline(other.pipeline),
		shaderStages(std::move(shaderStages)),
		bindingDescriptions(std::move(bindingDescriptions)),
		attributeDescriptions(std::move(attributeDescriptions))
	{
		other.pipelineLayout = nullptr;
		other.pipeline = nullptr;
	}

	VulkanPipeline::~VulkanPipeline()
	{
		Clean();
	}

	void VulkanPipeline::Clean()
	{
		shaderStages.clear();
		bindingDescriptions.clear();
		attributeDescriptions.clear();

		if (pipeline)
		{
			vkDestroyPipeline(device, pipeline, nullptr);
			pipeline = nullptr;
		}

		if (pipelineLayout)
		{
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			pipelineLayout = nullptr;
		}
	}

	auto VulkanPipeline::SetShader(VulkanShader* shader) -> VulkanPipeline&
	{
		this->shader = shader;
		return *this;
	}

	auto VulkanPipeline::AddShaderStage(ShaderStage stage) -> VulkanPipeline&
	{
		assert(shader);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		if (stage == ShaderStage::Vertex)
		{
			vertShaderStageInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = shader->GetVertexShader();
		}
		else if (stage == ShaderStage::Fragment)
		{
			vertShaderStageInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
			vertShaderStageInfo.module = shader->GetFragmentShader();
		}
		vertShaderStageInfo.pName = "main";

		shaderStages.push_back(vertShaderStageInfo);
		
		return *this;
	}

	auto VulkanPipeline::ResetShaderStage() -> VulkanPipeline&
	{
		shaderStages.clear();

		return *this;
	}

	auto VulkanPipeline::AddBindingDescription(const VkVertexInputBindingDescription& bindingDescription) -> VulkanPipeline&
	{
		bindingDescriptions.push_back(bindingDescription);

		return *this;
	}

	auto VulkanPipeline::ResetBindingDescription() -> VulkanPipeline&
	{
		bindingDescriptions.clear();

		return *this;
	}

	auto VulkanPipeline::AddAttributeDescription(const VkVertexInputAttributeDescription& attributeDescription) -> VulkanPipeline&
	{
		attributeDescriptions.push_back(attributeDescription);

		return *this;
	}

	SH_RENDER_API auto VulkanPipeline::ResetAttributeDescription() -> VulkanPipeline&
	{
		attributeDescriptions.clear();

		return *this;
	}

	auto VulkanPipeline::Build() -> VkResult
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return result;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();;
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data(); //버텍스 바인딩
		vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // 버텍스 어트리뷰트

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(viewportX);
		viewport.height = static_cast<float>(viewportY);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkExtent2D scissorExtend;
		scissorExtend.width = viewportX;
		scissorExtend.height = viewportY;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = scissorExtend;

		VkDynamicState dynamicStates[] = {
			VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
			VkDynamicState::VK_DYNAMIC_STATE_SCISSOR,
			//VkDynamicState::VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;
		
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		//viewportState.pViewports = &viewport; //동적 state이므로 지금 추가 안 해도 된다.
		viewportState.scissorCount = 1;
		//viewportState.pScissors = &scissor; //동적 state이므로 지금 추가 안 해도 된다.

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL; //채우기
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = 
			VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT | 
			VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT |
			VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT |
			VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		//기본 공식
		//finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
		//finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);

		//현재 공식(알파 블렌딩)
		//finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
		//finalColor.a = newAlpha.a;
		colorBlendAttachment.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional
		
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = nullptr; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		result = vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo, nullptr, &pipeline);
		assert(result == VkResult::VK_SUCCESS);
		return result;
	}

	auto VulkanPipeline::GetPipeline() const -> VkPipeline
	{
		return pipeline;
	}
}