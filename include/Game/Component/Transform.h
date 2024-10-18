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
		SCLASS(Transform)
	private:
		PROPERTY(vPosition)
		glm::vec3 vPosition;
		PROPERTY(vScale)
		glm::vec3 vScale;
		PROPERTY(vRotation)
		glm::vec3 vRotation;

		glm::vec3 worldPosition;
		glm::vec3 worldRotation;
		glm::vec3 worldScale;

		glm::mat4 matModel;

		glm::quat quat;
		glm::quat worldQuat;

		Transform* parent;
		PROPERTY(childs)
		core::SVector<Transform*> childs;

		bool bUpdateMatrix;
	private:
		void UpdateMatrix();
		void RemoveChild(const Transform& child);
	public:
		const glm::vec3& position;
		const glm::vec3& scale;
		const glm::vec3& rotation;
		const glm::mat4& localToWorldMatrix;
	public:
		SH_GAME_API Transform(GameObject& owner);
		SH_GAME_API ~Transform();
		SH_GAME_API Transform(const Transform& other);
		SH_GAME_API Transform(Transform&& other) noexcept;

		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void Update() override;

		SH_GAME_API auto GetParent() const -> Transform*;
		SH_GAME_API auto GetChildren() const -> const core::SVector<Transform*>&;
		/// @brief 자식 객체를 배열상에서 한칸 왼쪽으로 미는 함수. 제일 왼쪽이면 아무 일도 일어나지 않는다.
		SH_GAME_API void ReorderChildAbove(Transform* child);
		SH_GAME_API void SetPosition(const glm::vec3& pos);
		SH_GAME_API void SetPosition(float x, float y, float z);
		SH_GAME_API void SetScale(const glm::vec3& scale);
		SH_GAME_API void SetScale(float x, float y, float z);
		SH_GAME_API void SetScale(float scale);
		SH_GAME_API void SetRotation(const glm::vec3& rot);
		SH_GAME_API void SetRotation(const glm::quat& rot);

		SH_GAME_API auto GetQuat() const -> const glm::quat&;
		SH_GAME_API auto GetWorldQuat() const -> const glm::quat&;
		SH_GAME_API auto GetWorldPosition() const -> const glm::vec3&;
		SH_GAME_API auto GetWorldRotation() const -> const glm::vec3&;
		SH_GAME_API auto GetWorldScale() const -> const glm::vec3&;
		SH_GAME_API auto GetWorldToLocalMatrix() const -> glm::mat4;

		SH_GAME_API void SetParent(Transform* parent);
		SH_GAME_API bool HasChild(const Transform& child) const;

		SH_GAME_API void OnDestroy() override;
#if SH_EDITOR
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& property) override;
#endif
	};
}