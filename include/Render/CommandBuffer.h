#pragma once
#include "Core/NonCopyable.h"
#include "Core/Name.h"

#include "Render/IRenderThrMethod.h"

#include <array>
namespace sh::render
{
	class ComputeShader;
	class RenderTexture;
	class IBuffer;
	class Material;
	class Drawable;
	class ShaderPass;
	struct RenderData;
	struct BarrierInfo;

	class CommandBuffer : public core::INonCopyable
	{
		friend struct IRenderThrMethod<CommandBuffer>;
	public:
		virtual ~CommandBuffer() = default;

		virtual void Begin(bool bOnetime) = 0;
		virtual void End() = 0;
		virtual void Reset() = 0;

		virtual void Blit(RenderTexture& src, int x, int y, IBuffer& dst) = 0;
		virtual void Dispatch(const ComputeShader& shader, uint32_t x, uint32_t y, uint32_t z) = 0;
		virtual void SetRenderData(const RenderData& renderData, bool bClearColor = true, bool bClearDepth = true, bool bStoreColor = false, bool bStoreDepth = false) = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void DrawMeshBatch(const std::vector<const Drawable*>& drawables, core::Name passName, std::size_t viewerIdx = 0) = 0;
		virtual void DrawMesh(const Drawable& drawable, core::Name passName, std::size_t viewerIdx = 0) = 0;
		virtual void EmitBarrier(const std::vector<BarrierInfo>& barriers) = 0;
	protected:
		virtual auto GetRenderCall() const -> uint32_t = 0;
	};

	template<>
	struct IRenderThrMethod<CommandBuffer>
	{
		static auto GetRenderCall(const CommandBuffer& cmd) -> uint32_t { return cmd.GetRenderCall(); }
	};
}//namespace