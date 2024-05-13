#include "VulkanDrawable.h"

#include "VulkanRenderer.h"
#include "Material.h"
#include "Mesh.h"
#include "VulkanShader.h"
#include "VulkanImpl/VulkanFramebuffer.h"

#include <cstring>

namespace sh::render
{
	VulkanDrawable::VulkanDrawable(const VulkanRenderer& renderer) :
		renderer(renderer), vertexBuffer(renderer.GetDevice(), renderer.GetGPU())
	{
		auto frameBuffer = static_cast<const impl::VulkanFramebuffer*>(renderer.GetMainFramebuffer());
		pipeline = std::make_unique<impl::VulkanPipeline>(renderer.GetDevice(), frameBuffer->GetRenderPass());
	}

	VulkanDrawable::~VulkanDrawable()
	{
		
	}

	auto VulkanDrawable::GetPipeline() const -> impl::VulkanPipeline*
	{
		return pipeline.get();
	}

	void VulkanDrawable::Build(Material* mat, Mesh* mesh)
	{
		this->mat = mat;
		this->mesh = mesh;
		VkVertexInputBindingDescription bindingDesc{};
		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(glm::vec3);
		bindingDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attrDesc{};
		attrDesc.binding = 0;
		attrDesc.location = 0;
		attrDesc.format = VkFormat::VK_FORMAT_R32G32_SFLOAT;
		attrDesc.offset = 0;

		auto shader = mat->GetShader();
		pipeline->
			SetShader(static_cast<VulkanShader*>(shader)).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Vertex).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Fragment).
			AddBindingDescription(bindingDesc).
			AddAttributeDescription(attrDesc).
			Build();

		size_t size = sizeof(glm::vec3) * mesh->GetVertexCount();
		int flag = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		vertexBuffer.Create(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, flag);
		vertexBuffer.SetData(mesh->GetVertex().data());
	}

	auto VulkanDrawable::GetVertexBuffer() const -> const impl::VulkanBuffer&
	{
		return vertexBuffer;
	}

	void VulkanDrawable::Update()
	{
		vertexBuffer.SetData(mesh->GetVertex().data());
	}
}