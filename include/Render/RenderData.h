#pragma once
#include "Mesh.h"
#include "CommandBuffer.h"

#include "Core/SContainer.hpp"

#include <cstdint>
#include <functional>
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
		struct Group
		{
			const Material* material;
			Mesh::Topology topology;
			std::vector<Drawable*> drawables;
		};
		std::vector<Group> groups;
		std::vector<std::function<void(CommandBuffer&)>> drawCall;

		bool bClearColor = true;
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