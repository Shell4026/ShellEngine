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

		bool bClearColor = true;
		bool bClearDepth = true;
	};

	enum class ImageUsage
	{
		Undefined,
		ColorAttachment,
		SampledRead,
		Present,
		Src
	};

	struct BarrierInfo
	{
		std::variant<const RenderTexture*, uint32_t> target; // uint32_t = 스왑체인
		ImageUsage lastUsage;
		ImageUsage curUsage;
	};
}//namespace