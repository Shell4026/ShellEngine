#pragma once
#include "Mesh.h"
#include "Texture.h"

#include "Core/SContainer.hpp"
#include "Core/Name.h"

#include <cstdint>
#include <functional>
#include <variant>

namespace sh
{
	template<typename T>
	using Ref = std::reference_wrapper<T>;
}
namespace sh::render
{
	class Camera;
	class RenderTexture;
	class Drawable;
	class Material;
	class CommandBuffer;

	struct RenderViewer
	{
		glm::mat4 viewMatrix;
		glm::mat4 projMatrix;
		glm::vec3 pos;
		glm::vec3 to;

		glm::uvec4 viewportRect;
		glm::uvec4 viewportScissor;
		std::size_t offset = 0;
	};

	struct RenderData
	{
		int priority = 0;
		uint32_t frameIndex = 0;
		const RenderTexture* target = nullptr;
		core::Name tag{ "Camera" };

		std::vector<RenderViewer> renderViewers;
		const std::vector<Drawable*>* drawables = nullptr;
	};

	struct RenderTargetLayout
	{
		TextureFormat format;
		TextureFormat depthFormat;
		bool bUseMSAA = false;

		auto operator==(const RenderTargetLayout& other) const -> bool
		{
			return format == other.format && depthFormat == other.depthFormat && bUseMSAA == other.bUseMSAA;
		}
	};

	//struct DrawList
	//{
	//	struct RenderGroup
	//	{
	//		const Material* material;
	//		Mesh::Topology topology;
	//		std::vector<Drawable*> drawables;
	//	};
	//	struct RenderItem
	//	{
	//		const Material* material;
	//		Mesh::Topology topology;
	//		Drawable* drawable;
	//	};
	//	std::variant<std::vector<RenderGroup>, std::vector<RenderItem>> renderData;
	//	uint32_t drawableCount = 0;
	//};

	/// @brief 리소스(이미지/버퍼) 사용 의도. 배리어 계산의 입력으로 쓰임
	enum class ResourceUsage
	{
		Undefined,
		ColorAttachment,
		SampledRead,
		Present,
		TransferSrc,
		DepthStencilAttachment,
		DepthStencilSampledRead
	};

	struct BarrierInfo
	{
		std::variant<const RenderTexture*, uint32_t> target; // uint32_t = 스왑체인
		ResourceUsage lastUsage;
		ResourceUsage curUsage;
	};
}//namespace

namespace std
{
	template<>
	struct hash<sh::render::RenderTargetLayout>
	{
		auto operator()(const sh::render::RenderTargetLayout& layout) const -> std::size_t
		{
			std::hash<int> intHasher{};
			std::hash<int> boolHasher{};

			std::size_t hash = intHasher(static_cast<int>(layout.format));
			hash = sh::core::Util::CombineHash(hash, intHasher(static_cast<int>(layout.depthFormat)));
			hash = sh::core::Util::CombineHash(hash, boolHasher(layout.bUseMSAA));

			return hash;
		}
	};
}//namespace std
