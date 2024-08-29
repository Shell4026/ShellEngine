#pragma once

#include "Export.h"

#include "Core/SObject.h"
#include "Core/NonCopyable.h"

#include <vector>

namespace sh::render
{
	class Mesh;
	class Material;
	class Texture;
	class Framebuffer;

	class IDrawable : public core::SObject, public core::INonCopyable
	{
	public:
		enum class Stage
		{
			Vertex,
			Fragment
		};
	public:
		SH_RENDER_API virtual ~IDrawable() = default;
		SH_RENDER_API virtual void Build(Mesh* mesh, Material* mat) = 0;

		SH_RENDER_API virtual auto GetMaterial() const -> Material* = 0;
		SH_RENDER_API virtual auto GetMesh() const-> Mesh* = 0;

		/// @brief [게임 스레드용] 유니폼에 데이터를 지정한다.
		/// @param binding 바인딩 번호
		/// @param data 데이터 위치 포인터
		/// @param stage 셰이더 스테이지
		/// @return 
		SH_RENDER_API virtual void SetUniformData(uint32_t binding, const void* data, Stage stage) = 0;

		/// @brief [게임 스레드용] 유니폼 텍스쳐 데이터를 지정한다.
		/// @param binding 텍스쳐 바인딩 번호
		/// @param tex 텍스쳐 포인터
		/// @return 
		SH_RENDER_API virtual void SetTextureData(uint32_t binding, Texture* tex) = 0;

		SH_RENDER_API virtual void SetFramebuffer(Framebuffer& framebuffer) = 0;
		SH_RENDER_API virtual auto GetFramebuffer() const -> const Framebuffer* = 0;

		SH_RENDER_API virtual void SyncGameThread() = 0;
	};
}