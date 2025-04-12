#include "VulkanPipeline.h"
#include "VulkanShaderPass.h"

#include <cassert>
namespace sh::render::vk
{
	SH_RENDER_API VulkanPipeline::VulkanPipeline(VkDevice device, VkRenderPass renderPass) :
		device(device), renderPass(renderPass),
		pipeline(nullptr), shader(nullptr),
		cullMode(CullMode::Back),
		topology(Topology::Triangle)
	{
		stencilState.reference = 0;
		stencilState.compareMask = 0xFF;
		stencilState.compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS;
		stencilState.passOp = VkStencilOp::VK_STENCIL_OP_KEEP;
		stencilState.failOp = VkStencilOp::VK_STENCIL_OP_KEEP;
		stencilState.depthFailOp = VkStencilOp::VK_STENCIL_OP_KEEP;
		stencilState.writeMask = 0xFF;
	}

	SH_RENDER_API VulkanPipeline::VulkanPipeline(VulkanPipeline&& other) noexcept :
		pipeline(other.pipeline),
		device(other.device), renderPass(other.renderPass), shader(other.shader),
		shaderStages(std::move(other.shaderStages)),
		bindingDescriptions(std::move(other.bindingDescriptions)),
		attributeDescriptions(std::move(other.attributeDescriptions)),
		cullMode(other.cullMode),
		topology(other.topology),
		stencilState(other.stencilState),
		lineWidth(other.lineWidth),
		sampleCount(other.sampleCount),
		bUseStencil(other.bUseStencil),
		bZWrite(other.bZWrite)
	{
		other.pipeline = nullptr;
		other.device = nullptr;
		other.renderPass = nullptr;
		other.shader = nullptr;
	}

	SH_RENDER_API VulkanPipeline::~VulkanPipeline()
	{
		Clean();
	}

	SH_RENDER_API void VulkanPipeline::Clean()
	{
		lineWidth = 1.f;
		bUseStencil = false;
		shaderStages.clear();
		bindingDescriptions.clear();
		attributeDescriptions.clear();

		if (pipeline != nullptr)
		{
			vkDestroyPipeline(device, pipeline, nullptr);
			pipeline = nullptr;
		}
	}

	SH_RENDER_API auto VulkanPipeline::SetShader(const VulkanShaderPass* shader) -> VulkanPipeline&
	{
		this->shader = shader;
		return *this;
	}

	SH_RENDER_API auto VulkanPipeline::AddShaderStage(ShaderStage stage) -> VulkanPipeline&
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

	SH_RENDER_API auto VulkanPipeline::ResetShaderStage() -> VulkanPipeline&
	{
		shaderStages.clear();

		return *this;
	}

	SH_RENDER_API auto VulkanPipeline::AddBindingDescription(const VkVertexInputBindingDescription& bindingDescription) -> VulkanPipeline&
	{
		bindingDescriptions.push_back(bindingDescription);

		return *this;
	}

	SH_RENDER_API auto VulkanPipeline::ResetBindingDescription() -> VulkanPipeline&
	{
		bindingDescriptions.clear();

		return *this;
	}

	SH_RENDER_API auto VulkanPipeline::AddAttributeDescription(const VkVertexInputAttributeDescription& attributeDescription) -> VulkanPipeline&
	{
		attributeDescriptions.push_back(attributeDescription);

		return *this;
	}

	SH_RENDER_API auto VulkanPipeline::ResetAttributeDescription() -> VulkanPipeline&
	{
		attributeDescriptions.clear();

		return *this;
	}

	SH_RENDER_API auto VulkanPipeline::Build(VkPipelineLayout layout) -> VkResult
	{
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data(); // 버텍스 버퍼 바인딩
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // 버텍스 어트리뷰트

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		switch (topology)
		{
		case Topology::Point:
			inputAssembly.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			break;
		case Topology::Line:
			inputAssembly.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			break;
		}

		VkDynamicState dynamicStates[] = 
		{
			VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
			VkDynamicState::VK_DYNAMIC_STATE_SCISSOR,
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
		rasterizer.lineWidth = lineWidth;
		rasterizer.frontFace = VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;
		switch (cullMode)
		{
		case CullMode::Off:
			rasterizer.cullMode = VkCullModeFlagBits::VK_CULL_MODE_NONE;
			break;
		case CullMode::Front:
			rasterizer.cullMode = VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT;
			break;
		case CullMode::Back:
			rasterizer.cullMode = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
			break;
		}

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.rasterizationSamples = sampleCount;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = shader->GetColorMask();
			//VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT | 
			//VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT |
			//VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT |
			//VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT;
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
		colorBlending.logicOp = VkLogicOp::VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = true;
		depthStencil.depthWriteEnable = bZWrite;
		depthStencil.depthCompareOp = VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencil.depthBoundsTestEnable = false;
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		depthStencil.stencilTestEnable = bUseStencil;
		depthStencil.front = stencilState;
		depthStencil.back = stencilState;
		
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = layout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = nullptr;
		pipelineInfo.basePipelineIndex = -1;

		auto result = vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo, nullptr, &pipeline);
		assert(result == VkResult::VK_SUCCESS);
		return result;
	}

	SH_RENDER_API auto VulkanPipeline::GetPipeline() const -> VkPipeline
	{
		return pipeline;
	}

	SH_RENDER_API auto VulkanPipeline::SetCullMode(CullMode mode) -> VulkanPipeline&
	{
		cullMode = mode;
		return *this;
	}
	SH_RENDER_API auto sh::render::vk::VulkanPipeline::GetCullMode() const -> CullMode
	{
		return cullMode;
	}
	SH_RENDER_API auto VulkanPipeline::SetTopology(Topology topology) -> VulkanPipeline&
	{
		this->topology = topology;
		return *this;
	}
	SH_RENDER_API auto VulkanPipeline::GetTopology() const -> Topology
	{
		return topology;
	}

	SH_RENDER_API void VulkanPipeline::SetLineWidth(float width)
	{
		lineWidth = width;
	}
	SH_RENDER_API auto VulkanPipeline::GetLineWidth() const -> float
	{
		return lineWidth;
	}

	SH_RENDER_API auto VulkanPipeline::SetStencilState(bool bUse, VkStencilOpState stencilState) -> VulkanPipeline&
	{
		bUseStencil = bUse;
		this->stencilState = stencilState;

		return *this;
	}
	SH_RENDER_API auto VulkanPipeline::GetStencilState() const -> const VkStencilOpState&
	{
		return stencilState;
	}
	SH_RENDER_API auto VulkanPipeline::SetZWrite(bool zwrite) -> VulkanPipeline&
	{
		bZWrite = zwrite;
		return *this;
	}
	SH_RENDER_API auto VulkanPipeline::GetZWrite() const -> bool
	{
		return bZWrite;
	}
	SH_RENDER_API auto VulkanPipeline::SetSampleCount(VkSampleCountFlagBits sampleCount) -> VulkanPipeline&
	{
		this->sampleCount = sampleCount;
		return *this;
	}
	SH_RENDER_API auto VulkanPipeline::GetSampleCount() const -> VkSampleCountFlagBits
	{
		return sampleCount;
	}
}