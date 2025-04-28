#pragma once
#include "Component.h"
#include "Game/Export.h"

#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Render/MaterialPropertyBlock.h"

#include "Core/Util.h"
#include "Core/Observer.hpp"

#include "glm/mat4x4.hpp"
#include <unordered_map>
#include <memory>
#include <vector>
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
		render::AABB worldAABB;
		
		PROPERTY(propertyBlock, core::PropertyOption::invisible)
		render::MaterialPropertyBlock* propertyBlock = nullptr;

		std::vector<std::pair<const render::ShaderPass*, const render::UniformStructLayout*>> localUniformLocations;

		core::Observer<false, const glm::mat4&>::Listener onMatrixUpdateListener;

		PROPERTY(renderTag)
		uint32_t renderTag = 1;

		bool bShaderHasLight = false;
	protected:
		PROPERTY(mesh)
		const render::Mesh* mesh;
		PROPERTY(mat)
		render::Material* mat;
		PROPERTY(drawable, core::PropertyOption::invisible, core::PropertyOption::noSave)
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
		SH_GAME_API virtual void CreateDrawable();
		/// @brief Update()마다 호출 되는 Drawable 설정 함수
		SH_GAME_API virtual void UpdateDrawable();
	public:
		SH_GAME_API MeshRenderer(GameObject& owner);
		SH_GAME_API ~MeshRenderer();

		SH_GAME_API void SetMesh(const render::Mesh* mesh);
		SH_GAME_API auto GetMesh() const -> const render::Mesh*;

		SH_GAME_API void SetMaterial(render::Material* mat);
		SH_GAME_API auto GetMaterial() const -> render::Material*;

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