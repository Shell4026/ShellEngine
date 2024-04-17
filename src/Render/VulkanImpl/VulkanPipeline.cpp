#include "VulkanPipeline.h"

#include "VulkanShader.h"
#include "VulkanSurface.h"

#include <cassert>
namespace sh::render::impl
{
	VulkanPipeline::VulkanPipeline(const VulkanSurface& surface, const VulkanShader* shader) :
		surface(surface),
		renderPass(nullptr), pipelineLayout(nullptr), pipeline(nullptr), device(surface.GetDevice()),
		shader(shader)
	{
	}

	VulkanPipeline::~VulkanPipeline()
	{
		Destroy();
	}
	void VulkanPipeline::Destroy()
	{
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

		if (renderPass)
		{
			vkDestroyRenderPass(device, renderPass, nullptr);
			renderPass = nullptr;
		}
	}

	void VulkanPipeline::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = surface.GetSwapChainImageFormat();
		colorAttachment.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		//스텐실
		colorAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//픽셀 셰이더에서 출력되는 attachment
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
		assert(result == VkResult::VK_SUCCESS);
	}

	void VulkanPipeline::AddShaderStage(ShaderStage stage)
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
	}

	auto VulkanPipeline::CreateGraphicsPipeline() -> VkResult
	{
		CreateRenderPass();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(surface.GetSwapChainSize().width);
		viewport.height = static_cast<float>(surface.GetSwapChainSize().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = surface.GetSwapChainSize();

		std::vector<VkDynamicState> dynamicStates =
		{
			VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
			VkDynamicState::VK_DYNAMIC_STATE_SCISSOR,
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();
		
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
		rasterizer.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL;
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

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
		assert(result == VkResult::VK_SUCCESS);
		if(result != VkResult::VK_SUCCESS)
			return result;
		
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

	auto VulkanPipeline::GetRenderPass() const -> VkRenderPass
	{
		return renderPass;
	}

	auto VulkanPipeline::GetPipeline() const -> VkPipeline
	{
		return pipeline;
	}

	auto VulkanPipeline::GetDevice() const -> VkDevice
	{
		return device;
	}
}