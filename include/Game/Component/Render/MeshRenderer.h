#pragma once
#include "Game/Export.h"
#include "Game/Component/Component.h"

#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Render/MaterialPropertyBlock.h"
#include "Render/Drawable.h"

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

		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		/// @brief 렌더러에 이 타이밍에 drawable 객체를 넣는다.
		SH_GAME_API void LateUpdate() override;
		SH_GAME_API void Deserialize(const core::Json& json) override;
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API void SetMesh(const render::Mesh* mesh);
		SH_GAME_API auto GetMesh() const -> const render::Mesh* { return mesh; }

		SH_GAME_API void SetMaterial(render::Material* mat);
		SH_GAME_API void SetMaterial(std::size_t index, render::Material* mat);
		SH_GAME_API auto GetMaterial(std::size_t index = 0) const -> render::Material*;
		SH_GAME_API auto GetMaterialCount() const -> std::size_t;

		SH_GAME_API void SetMaterialPropertyBlock(std::unique_ptr<render::MaterialPropertyBlock>&& block);
		SH_GAME_API void SetMaterialPropertyBlock(std::size_t index, std::unique_ptr<render::MaterialPropertyBlock>&& block);
		SH_GAME_API auto GetMaterialPropertyBlock(std::size_t index = 0) const -> render::MaterialPropertyBlock*;
		/// @brief PropertyBlock의 데이터를 GPU에 업로드 하는 함수
		SH_GAME_API void UpdatePropertyBlockData();

		SH_GAME_API void SetRenderTagId(uint32_t tagId);
		SH_GAME_API auto GetRenderTagId() const -> uint32_t { return renderTag; }

		SH_GAME_API auto GetWorldAABB() const -> const render::AABB { return worldAABB; }
	protected:
		/// @brief Drawable을 생성하거나 이미 존재 시 갱신하는 함수.
		SH_GAME_API virtual void CreateDrawable(bool bUseSubMesh = true);
		/// @brief Update()마다 호출 되는 Drawable 설정 함수
		SH_GAME_API virtual void UpdateDrawable();
		SH_GAME_API void SearchLocalProperties();
		SH_GAME_API void SetDefaultLocalProperties();
	private:
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

		void FillLightStruct(render::Drawable& drawable, render::Shader& shader);
	protected:
		PROPERTY(drawables, core::PropertyOption::invisible, core::PropertyOption::noSave)
		std::vector<render::Drawable*> drawables;
		PROPERTY(mesh)
		const render::Mesh* mesh;
		PROPERTY(mats)
		std::vector<render::Material*> mats;
	private:
		struct alignas(16) Light
		{
			glm::vec4 pos;
			glm::vec4 other;
			glm::mat4 lightSpaceMatrix;
		};
		std::vector<uint8_t> lightDatas;
		render::AABB worldAABB;

		std::vector<std::unique_ptr<render::MaterialPropertyBlock>> propertyBlocks;

		using LocalUniformLocations = std::vector<std::pair<const render::ShaderPass*, const render::UniformStructLayout*>>;
		std::vector<LocalUniformLocations> localUniformLocationsList;

		core::Observer<false, const glm::mat4&>::Listener onMatrixUpdateListener;
		core::Observer<false, const render::Shader*>::Listener onShaderChangedListener;

		PROPERTY(renderTag)
		uint32_t renderTag = 1;
	};
}//namespace
