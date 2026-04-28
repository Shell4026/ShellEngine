#pragma once
#include "Mesh.h"
#include "CommandBuffer.h"

#include "Core/SContainer.hpp"

#include <cstdint>
#include <functional>
#include <variant>
namespace sh::render
{
	class Camera;
	class RenderTexture;
	class Drawable;
	class Material;

	struct RenderTarget
	{
		uint32_t frameIndex;
		const Camera* camera;
		const RenderTexture* target;
		const std::vector<Drawable*>* drawables;
	};

	struct DrawList
	{
		struct RenderGroup
		{
			const Material* material;
			Mesh::Topology topology;
			std::vector<Drawable*> drawables;
		};
		struct RenderItem
		{
			const Material* material;
			Mesh::Topology topology;
			Drawable* drawable;
		};
		std::variant<std::vector<RenderGroup>, std::vector<RenderItem>> renderData;
		std::vector<std::function<void(CommandBuffer&)>> drawCall;

		uint32_t drawableCount = 0;

		bool bClearColor = true;
		bool bClearDepth = true;
	};

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