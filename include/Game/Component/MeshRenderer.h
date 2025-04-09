#pragma once
#include "Component.h"
#include "Game/Export.h"

#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Render/MaterialPropertyBlock.h"

#include "Core/Util.h"
#include "Core/SContainer.hpp"
#include "Core/Observer.hpp"

#include "glm/mat4x4.hpp"
#include <unordered_map>
#include <memory>

namespace sh::render
{
	class Drawable;
}
namespace sh::game
{
	class Camera;

	class MeshRenderer : public Component
	{
		COMPONENT(MeshRenderer)
	private:
		PROPERTY(propertyBlock, core::PropertyOption::invisible)
		PROPERTY(renderTag)
		PROPERTY(mesh)
		PROPERTY(mat)
		PROPERTY(drawable, core::PropertyOption::invisible)

		render::AABB worldAABB;
		
		render::MaterialPropertyBlock* propertyBlock = nullptr;

		core::SVector<std::pair<const render::ShaderPass*, const render::UniformStructLayout*>> localUniformLocations;

		core::Observer<false, const glm::mat4&>::Listener onMatrixUpdateListener;

		uint32_t renderTag = 1;

		bool bShaderHasLight = false;
	protected:
		render::Mesh* mesh;
		render::Material* mat;
		render::Drawable* drawable;
	private:
		void UpdateMaterialData();

		template<typename T>
		void SetData(const T& data, std::vector<uint8_t>& uniformData, std::size_t offset, std::size_t size)
		{
			std::memcpy(uniformData.data() + offset, &data, size);
		}
		template<typename T>
		void SetData(const T& data, std::vector<uint8_t>& uniformData, std::size_t offset)
		{
			std::memcpy(uniformData.data() + offset, &data, sizeof(T));
		}

		void FillLightStruct(render::Drawable& drawable) const;
	protected:
		/// @brief Drawable을 생성하거나 이미 존재 시 갱신하는 함수.
		void CreateDrawable();
	public:
		SH_GAME_API MeshRenderer(GameObject& owner);
		SH_GAME_API ~MeshRenderer();

		SH_GAME_API void SetMesh(sh::render::Mesh* mesh);
		SH_GAME_API auto GetMesh() const -> const sh::render::Mesh*;
		SH_GAME_API auto GetMesh() -> sh::render::Mesh*;

		SH_GAME_API void SetMaterial(sh::render::Material* mat);
		SH_GAME_API auto GetMaterial() const -> sh::render::Material*;

		SH_GAME_API void Destroy() override;
		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void Update() override;

		SH_GAME_API void SetMaterialPropertyBlock(render::MaterialPropertyBlock* block);
		SH_GAME_API auto GetMaterialPropertyBlock() const -> render::MaterialPropertyBlock*;

		SH_GAME_API void SetRenderTagId(uint32_t tagId);
		SH_GAME_API auto GetRenderTagId() const -> uint32_t;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;
	};
}