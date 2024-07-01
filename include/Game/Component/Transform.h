#pragma once

#include "Game/Export.h"
#include "Component.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace sh::game
{
	class Transform : public Component
	{
		SCOMPONENT(Transform)
	private:
		glm::vec3 localPosition;
		PROPERTY(vPosition)
		glm::vec3 vPosition;
		PROPERTY(vScale)
		glm::vec3 vScale;
		PROPERTY(vRot)
		glm::vec3 vRot;

		glm::mat4 matModel;

		glm::quat quat;

		bool bUpdateMatrix;
	private:
		void UpdateMatrix();
	public:
		const glm::vec3& position;
		const glm::vec3& scale;
		const glm::vec3& rotation;
		const glm::mat4& localToWorldMatrix;
	public:
		SH_GAME_API Transform();
		SH_GAME_API ~Transform();
		SH_GAME_API Transform(const Transform& other);
		SH_GAME_API Transform(Transform&& other) noexcept;

		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void Update() override;

		SH_GAME_API void SetPosition(const glm::vec3& pos);
		SH_GAME_API void SetPosition(float x, float y, float z);
		SH_GAME_API void SetScale(const glm::vec3& scale);
		SH_GAME_API void SetRotation(const glm::vec3& rot);
		SH_GAME_API void SetRotation(const glm::quat& rot);
#if SH_EDITOR
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& property) override;
#endif
	};
}