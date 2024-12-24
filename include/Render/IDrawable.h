#pragma once

#include "Export.h"
#include "Camera.h"

#include "Mesh.h"
#include "Material.h"
#include "Texture.h"

#include "Core/SObject.h"
#include "Core/NonCopyable.h"
#include "Core/ISyncable.h"

#include <vector>

namespace sh::render
{
	class Framebuffer;

	class IDrawable : 
		public core::SObject, 
		public core::INonCopyable, 
		public core::ISyncable
	{
	public:
		enum class Stage
		{
			Vertex,
			Fragment
		};
	public:
		SH_RENDER_API virtual ~IDrawable() = default;
		SH_RENDER_API virtual void Build(Camera& camera, Mesh* mesh, Material* mat) = 0;

		SH_RENDER_API virtual auto GetMaterial() const ->const Material* = 0;
		SH_RENDER_API virtual auto GetMesh() const -> const Mesh* = 0;
		SH_RENDER_API virtual auto GetCamera() const -> Camera* = 0;

		/// @brief [게임 스레드용] 로컬 유니폼에 데이터를 지정한다.
		/// @param binding 바인딩 번호
		/// @param data 데이터 위치 포인터
		/// @param stage 셰이더 스테이지
		SH_RENDER_API virtual void SetUniformData(uint32_t binding, const void* data, Stage stage) = 0;

		SH_RENDER_API virtual bool CheckAssetValid() const = 0;
	};
}