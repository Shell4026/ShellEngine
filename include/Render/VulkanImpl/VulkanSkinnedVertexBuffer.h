#pragma once
#include "../Export.h"
#include "../IVertexBuffer.h"
#include "VulkanBuffer.h"

#include <array>
#include <vector>

namespace sh::render::vk
{
	class VulkanContext;

	/// @brief SkinnedMesh용 버텍스 버퍼.
	/// binding 0: Mesh::Vertex (location 0-3)
	/// binding 1: SkinnedMesh::BoneVertex (location 4-5)
	class VulkanSkinnedVertexBuffer : public IVertexBuffer
	{
	public:
		SH_RENDER_API VulkanSkinnedVertexBuffer(const VulkanContext& context);
		SH_RENDER_API VulkanSkinnedVertexBuffer(const VulkanSkinnedVertexBuffer& other);
		SH_RENDER_API VulkanSkinnedVertexBuffer(VulkanSkinnedVertexBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanSkinnedVertexBuffer();

		SH_RENDER_API auto operator=(const VulkanSkinnedVertexBuffer& other) -> VulkanSkinnedVertexBuffer&;
		SH_RENDER_API auto operator=(VulkanSkinnedVertexBuffer&& other) noexcept -> VulkanSkinnedVertexBuffer&;

		/// @brief SkinnedMesh로 다운캐스트 후 두 버퍼를 생성한다.
		SH_RENDER_API void Create(const Mesh& mesh) override;
		SH_RENDER_API void Clear() override;

		SH_RENDER_API auto Clone() const -> std::unique_ptr<IVertexBuffer> override;

		SH_RENDER_API auto GetVertexBuffer() const -> const VulkanBuffer& { return vertexBuffer; }
		SH_RENDER_API auto GetBoneBuffer() const -> const VulkanBuffer& { return boneBuffer; }
		SH_RENDER_API auto GetIndexBuffer() const -> const VulkanBuffer& { return indexBuffer; }

		/// @brief binding 0 (Mesh::Vertex), binding 1 (BoneVertex) 두 개를 반환한다.
		SH_RENDER_API static auto GetBindingDescriptions() -> std::array<VkVertexInputBindingDescription, 2>;
		/// @brief location 0-5 (위치,UV,노말,탄젠트,boneIdx,boneWeight) 여섯 개를 반환한다.
		SH_RENDER_API static auto GetAttributeDescriptions() -> std::vector<VkVertexInputAttributeDescription>;
	private:
		void CreateBuffers(const Mesh& mesh);
	private:
		const VulkanContext& context;

		VulkanBuffer vertexBuffer; // binding 0
		VulkanBuffer boneBuffer;   // binding 1
		VulkanBuffer indexBuffer;

		SH_RENDER_API static inline std::vector<VkVertexInputAttributeDescription> attribDescriptions;
	};
}//namespace
