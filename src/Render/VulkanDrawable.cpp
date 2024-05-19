#include "VulkanDrawable.h"

#include "VulkanRenderer.h"
#include "Material.h"
#include "Mesh.h"
#include "VulkanShader.h"
#include "VulkanImpl/VulkanFramebuffer.h"
#include "VulkanUniform.h"

#include <cstring>

namespace sh::render
{
	VulkanDrawable::VulkanDrawable(const VulkanRenderer& renderer) :
		renderer(renderer), 
		indexBuffer(renderer.GetDevice(), renderer.GetGPU()),
		cmd(renderer.GetDevice(), renderer.GetCommandPool()),
		buffers(vertexBuffers),
		pipelineLayout(nullptr), mat(nullptr), mesh(nullptr),
		descriptorSetLayout(nullptr)
	{
		auto frameBuffer = static_cast<const impl::VulkanFramebuffer*>(renderer.GetMainFramebuffer());
		pipeline = std::make_unique<impl::VulkanPipeline>(renderer.GetDevice(), frameBuffer->GetRenderPass());
	}

	VulkanDrawable::~VulkanDrawable()
	{
		pipeline.reset();
		if (pipelineLayout)
		{
			vkDestroyPipelineLayout(renderer.GetDevice(), pipelineLayout, nullptr);
			pipelineLayout = nullptr;
		}
		if (descriptorSetLayout)
		{
			vkDestroyDescriptorSetLayout(renderer.GetDevice(), descriptorSetLayout, nullptr);
			descriptorSetLayout = nullptr;
		}
	}

	auto VulkanDrawable::GetPipelineLayout() const -> VkPipelineLayout
	{
		return pipelineLayout;
	}

	auto VulkanDrawable::GetPipeline() const -> impl::VulkanPipeline*
	{
		return pipeline.get();
	}

	void VulkanDrawable::CreateVertexBuffer()
	{
		VkVertexInputBindingDescription bindingDesc{};
		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(glm::vec3);
		bindingDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attrDesc{};
		attrDesc.binding = 0;
		attrDesc.location = 0;
		attrDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attrDesc.offset = 0;

		pipeline->
			AddBindingDescription(bindingDesc).
			AddAttributeDescription(attrDesc);

		size_t size = sizeof(glm::vec3) * mesh->GetVertexCount();
		impl::VulkanBuffer stagingBuffer1{ renderer.GetDevice(), renderer.GetGPU() };
		stagingBuffer1.Create(size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer1.SetData(mesh->GetVertex().data());

		vertexBuffers[0].Create(size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		size_t sizeIndices = sizeof(uint32_t) * mesh->GetIndices().size();
		impl::VulkanBuffer stagingBuffer2{ renderer.GetDevice(), renderer.GetGPU() };
		stagingBuffer2.Create(sizeIndices, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer2.SetData(mesh->GetIndices().data());

		indexBuffer.Create(sizeIndices,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkCommandBufferBeginInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		cmd.Submit(renderer.GetGraphicsQueue(), [&]() {
			VkBufferCopy cpy{};
			cpy.srcOffset = 0; // Optional
			cpy.dstOffset = 0; // Optional
			cpy.size = size;

			VkBufferCopy cpyIndices{};
			cpyIndices.srcOffset = 0; // Optional
			cpyIndices.dstOffset = 0; // Optional
			cpyIndices.size = sizeIndices;
			vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer1.GetBuffer(), vertexBuffers[0].GetBuffer(), 1, &cpy);
			vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer2.GetBuffer(), indexBuffer.GetBuffer(), 1, &cpyIndices);
			}, &info);
	}

	auto VulkanDrawable::CreatePipelineLayout() -> VkResult
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		return vkCreatePipelineLayout(renderer.GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	}

	auto VulkanDrawable::CreateDescriptorLayout(uint32_t binding) -> VkResult
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = 1;
		layoutBinding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBinding.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = 1;
		info.pBindings = &layoutBinding;

		return vkCreateDescriptorSetLayout(renderer.GetDevice(), &info, nullptr, &descriptorSetLayout);
	}

	void VulkanDrawable::Build(Material* mat, Mesh* mesh)
	{
		assert(mat);
		assert(mesh);

		this->mat = mat;
		this->mesh = mesh;

		Shader* shader = mat->GetShader();
		if (shader == nullptr)
			return;

		pipeline->Clean();

		cmd.Create();
		vertexBuffers.clear();
		indexBuffer.Clean();

		vertexBuffers.push_back(impl::VulkanBuffer{ renderer.GetDevice(), renderer.GetGPU() });
		CreateVertexBuffer();
		cmd.Clean();

		cmd.Create();

		int idx = 1;
		for (auto& attr : mesh->attributes)
		{
			auto shaderAttr = shader->GetAttribute(attr->name);
			if (!shaderAttr)
				continue;

			VkFormat format = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
			size_t size = attr->GetSize();
			const void* data = attr->GetData();

			switch (attr->GetStride())
			{
			case 4:
				if (attr->isInteger)
				{
					if (shaderAttr->type != Shader::DataType::Int)
						continue;
					format = VkFormat::VK_FORMAT_R32_SINT;
				}
				else
				{
					if (shaderAttr->type != Shader::DataType::Float)
						continue;
					format = VkFormat::VK_FORMAT_R32_SFLOAT;
				}
				break;
			case 8:
				if (shaderAttr->type != Shader::DataType::Vec2)
					continue;
				format = VkFormat::VK_FORMAT_R32G32_SFLOAT;
				break;
			case 12:
				if (shaderAttr->type != Shader::DataType::Vec3)
					continue;
				format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
				break;
			case 16:
				if (shaderAttr->type != Shader::DataType::Vec4)
					continue;
				format = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
				break;
			default:
				continue;
			}

			VkVertexInputBindingDescription bindingDesc{};
			bindingDesc.binding = idx;
			bindingDesc.stride = static_cast<uint32_t>(attr->GetStride());
			bindingDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

			VkVertexInputAttributeDescription attrDesc{};
			attrDesc.binding = idx;
			attrDesc.location = shaderAttr->idx;
			attrDesc.format = format;
			attrDesc.offset = 0;

			pipeline->
				AddBindingDescription(bindingDesc).
				AddAttributeDescription(attrDesc);
			
			impl::VulkanBuffer stagingBuffer{ renderer.GetDevice(), renderer.GetGPU() };
			stagingBuffer.Create(size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_SHARING_MODE_EXCLUSIVE,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBuffer.SetData(data);

			vertexBuffers.push_back(impl::VulkanBuffer{renderer.GetDevice(), renderer.GetGPU()});
			vertexBuffers.back().Create(size,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_SHARING_MODE_EXCLUSIVE,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			VkCommandBufferBeginInfo info{};
			info.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			cmd.Submit(renderer.GetGraphicsQueue(), [&]() {
				VkBufferCopy cpy{};
				cpy.srcOffset = 0; // Optional
				cpy.dstOffset = 0; // Optional
				cpy.size = size;
				vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer.GetBuffer(), vertexBuffers.back().GetBuffer(), 1, &cpy);
				}, &info);

			++idx;
		}

		auto result = CreateDescriptorLayout(0);
		assert(result == VkResult::VK_SUCCESS);
		result = CreatePipelineLayout();
		assert(result == VkResult::VK_SUCCESS);

		std::vector<VkDescriptorSetLayout> layouts(VulkanRenderer::MAX_FRAME_DRAW, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = renderer.GetDescriptorPool();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanRenderer::MAX_FRAME_DRAW);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(VulkanRenderer::MAX_FRAME_DRAW);
		result = vkAllocateDescriptorSets(renderer.GetDevice(), &allocInfo, descriptorSets.data());
		assert(result == VkResult::VK_SUCCESS);

		for (int i = 0; i < VulkanRenderer::MAX_FRAME_DRAW; ++i)
		{
			uniformBuffers.push_back(impl::VulkanBuffer{ renderer.GetDevice(), renderer.GetGPU() });
			result = uniformBuffers.back().Create(sizeof(glm::vec4),
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
				VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				true);
			assert(result == VkResult::VK_SUCCESS);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i].GetBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(glm::vec4);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(renderer.GetDevice(), 1, &descriptorWrite, 0, nullptr);
		}

		pipeline->
			SetShader(static_cast<VulkanShader*>(shader)).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Vertex).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Fragment).
			Build(pipelineLayout);

		cmd.Clean();
	}

	void VulkanDrawable::SetUniformData(int frame, const void* data)
	{
		uniformBuffers[frame].SetData(data);
	}

	auto VulkanDrawable::GetVertexBuffer() const -> const impl::VulkanBuffer&
	{
		return vertexBuffers[0];
	}

	auto VulkanDrawable::GetIndexBuffer() const -> const impl::VulkanBuffer&
	{
		return indexBuffer;
	}

	auto VulkanDrawable::GetMaterial() const -> Material*
	{
		return mat;
	}

	auto VulkanDrawable::GetMesh() const -> Mesh*
	{
		return mesh;
	}

	auto VulkanDrawable::GetDescriptorSet(int frame) -> VkDescriptorSet
	{
		return descriptorSets[frame];
	}
}