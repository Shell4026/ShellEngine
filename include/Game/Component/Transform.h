#pragma once

#include "Game/Export.h"
#include "Component.h"

#include "Game/Vector.h"
#include <glm/gtc/quaternion.hpp>

namespace sh::game
{
	class Transform : public Component
	{
		SCLASS(Transform)
	private:
		PROPERTY(vPosition)
		Vec3 vPosition;
		PROPERTY(vScale)
		Vec3 vScale;
		PROPERTY(vRotation)
		Vec3 vRotation;

		Vec3 worldPosition;
		Vec3 worldRotation;
		Vec3 worldScale;

		glm::mat4 matModel;

		glm::quat quat;
		glm::quat worldQuat;

		Transform* parent;
		PROPERTY(childs, core::PropertyOption::invisible)
		core::SVector<Transform*> childs;

		bool bUpdateMatrix;
	private:
		void UpdateMatrix();
		void RemoveChild(const Transform& child);
	public:
		const Vec3& position;
		const Vec3& scale;
		const Vec3& rotation;
		const glm::mat4& localToWorldMatrix;

		mutable core::Observer<false, const glm::mat4&> onMatrixUpdate;
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
		/// @brief 자식 객체를 배열상에서 한칸 왼쪽으로 미는 함수
		/// @brief 제일 왼쪽이면 아무 일도 일어나지 않는다.
		SH_GAME_API void ReorderChildAbove(Transform* child);
		SH_GAME_API void SetPosition(const Vec3& pos);
		SH_GAME_API void SetPosition(float x, float y, float z);
		SH_GAME_API void SetScale(const Vec3& scale);
		SH_GAME_API void SetScale(float x, float y, float z);
		SH_GAME_API void SetScale(float scale);
		SH_GAME_API void SetRotation(const Vec3& rot);
		SH_GAME_API void SetRotation(const glm::quat& rot);

		SH_GAME_API auto GetQuat() const -> const glm::quat&;
		SH_GAME_API auto GetWorldQuat() const -> const glm::quat&;
		SH_GAME_API auto GetWorldPosition() const -> const Vec3&;
		SH_GAME_API auto GetWorldRotation() const -> const Vec3&;
		SH_GAME_API auto GetWorldScale() const -> const Vec3&;
		SH_GAME_API auto GetWorldToLocalMatrix() const -> glm::mat4;

		SH_GAME_API void SetParent(Transform* parent);
		SH_GAME_API bool HasChild(const Transform& child) const;

		SH_GAME_API void OnDestroy() override;

		SH_GAME_API auto Serialize() const -> core::Json override;
		SH_GAME_API void Deserialize(const core::Json& json) override;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& property) override;
	};
}