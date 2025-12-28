#pragma once
#include "Component.h"
#include "Game/Export.h"

#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Render/MaterialPropertyBlock.h"
#include "Render/Drawable.h"

#include "Core/Util.h"
#include "Core/Observer.hpp"

#include "glm/mat4x4.hpp"
#include <unordered_map>
#include <memory>
#include <vector>
namespace sh::game
{
	class Camera;

	class MeshRenderer : public Component
	{
		COMPONENT(MeshRenderer)
	public:
		SH_GAME_API MeshRenderer(GameObject& owner);
		SH_GAME_API ~MeshRenderer();

		SH_GAME_API void SetMesh(const render::Mesh* mesh);
		SH_GAME_API auto GetMesh() const -> const render::Mesh*;

		SH_GAME_API void SetMaterial(render::Material* mat);
		SH_GAME_API auto GetMaterial() const -> render::Material*;

		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void LateUpdate() override;

		SH_GAME_API void SetMaterialPropertyBlock(std::unique_ptr<render::MaterialPropertyBlock>&& block);
		SH_GAME_API auto GetMaterialPropertyBlock() const -> render::MaterialPropertyBlock*;
		/// @brief PropertyBlock의 데이터를 GPU에 업로드 하는 함수
		SH_GAME_API void UpdatePropertyBlockData();

		SH_GAME_API void SetRenderTagId(uint32_t tagId);
		SH_GAME_API auto GetRenderTagId() const -> uint32_t;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API auto GetWorldAABB() const -> const render::AABB { return worldAABB; }
	protected:
		/// @brief Drawable을 생성하거나 이미 존재 시 갱신하는 함수.
		SH_GAME_API virtual void CreateDrawable();
		/// @brief Update()마다 호출 되는 Drawable 설정 함수
		SH_GAME_API virtual void UpdateDrawable();
	private:
		void SearchLocalProperties();

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

		void FillLightStruct(render::Drawable& drawable, render::Shader& shader) const;
	protected:
		PROPERTY(mesh)
		const render::Mesh* mesh;
		PROPERTY(mat)
		render::Material* mat;
		PROPERTY(drawable, core::PropertyOption::invisible, core::PropertyOption::noSave)
		render::Drawable* drawable;
	private:
		struct Light
		{
			alignas(16) int lightCount = 0;
			alignas(16) glm::vec4 lightPos[10];
			alignas(16) glm::vec4 other[10];
		};
		render::AABB worldAABB;

		std::unique_ptr<render::MaterialPropertyBlock> propertyBlock;

		std::vector<std::pair<const render::ShaderPass*, const render::UniformStructLayout*>> localUniformLocations;

		core::Observer<false, const glm::mat4&>::Listener onMatrixUpdateListener;

		PROPERTY(renderTag)
		uint32_t renderTag = 1;
	};
}//namespace