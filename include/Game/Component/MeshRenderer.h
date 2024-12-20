﻿#pragma once

#include "Component.h"

#include "Game/Export.h"

#include "Render/IDrawable.h"
#include "Render/Material.h"
#include "Render/Mesh.h"

#include "Core/Util.h"
#include "Core/SContainer.hpp"
#include "Core/Observer.hpp"

#include "glm/mat4x4.hpp"
#include <unordered_map>
#include <memory>

namespace sh::game
{
	class Camera;

	class MeshRenderer : public Component
	{
		COMPONENT(MeshRenderer);
	private:
		core::SVector<unsigned char> uniformCopyData;

		core::Observer<false, Camera*>::Listener onCameraAddListener;
		core::Observer<false, Camera*>::Listener onCameraRemoveListener;
		core::Observer<false, const glm::mat4&>::Listener onMatrixUpdateListener;

		render::AABB worldAABB;

		bool bShaderHasLight = false;
	protected:
		PROPERTY(mesh);
		const sh::render::Mesh* mesh;
		PROPERTY(mat);
		sh::render::Material* mat;
		PROPERTY(drawables, core::PropertyOption::invisible);
		core::SMap<Camera*, sh::render::IDrawable*> drawables;
	private:
		template <typename T>
		void SetUniformData(const T& data, std::vector<unsigned char>& uniformData, size_t offset);
	protected:
		/// @brief Drawable을 생성하거나 이미 존재 시 갱신하는 함수.
		/// @param camera 카메라 포인터
		void CreateDrawable(Camera* camera);

		/// @brief 모든 Drawable들을 갱신하는 함수.
		void RebuildDrawables();
		void CleanDrawables();
	public:
		SH_GAME_API MeshRenderer(GameObject& owner);
		SH_GAME_API ~MeshRenderer();

		SH_GAME_API void SetMesh(const sh::render::Mesh* mesh);
		SH_GAME_API auto GetMesh() const -> const sh::render::Mesh*;

		SH_GAME_API void SetMaterial(sh::render::Material* mat);
		SH_GAME_API auto GetMaterial() const -> sh::render::Material*;

		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void Update() override;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;
	};

	template<typename T>
	void MeshRenderer::SetUniformData(const T& data, std::vector<unsigned char>& uniformData, size_t offset)
	{
		std::memcpy(uniformData.data() + offset, &data, sizeof(T));
	}
}