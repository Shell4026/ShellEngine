﻿#include "PCH.h"
#include "Component/Transform.h"
#include "GameObject.h"

#include "glm/mat4x4.hpp"
#include <algorithm>

namespace sh::game
{
	SH_GAME_API Transform::Transform(GameObject& owner) :
		Component(owner),
		position(vPosition), scale(vScale), rotation(vRotation), localToWorldMatrix(matModel),

		worldPosition(), worldRotation(), worldScale(),
		vPosition{ 0.f, 0.f, 0.f }, vScale{ 1.0f, 1.0f, 1.0f }, vRotation{ 0.f, 0.f, 0.f },
		matModel(), quat(glm::radians(glm::vec3{ vRotation })),
		parent(nullptr), childs(),
		bUpdateMatrix(false)
	{
		matModel = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ vPosition }) * glm::mat4_cast(quat) * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ vScale });
	}
	SH_GAME_API Transform::~Transform()
	{

	}

	SH_GAME_API Transform::Transform(const Transform& other) :
		Component(other),
		position(vPosition), scale(vScale), rotation(vRotation), localToWorldMatrix(matModel),

		worldPosition(other.worldPosition), worldRotation(other.worldRotation), worldScale(other.worldScale),
		vPosition(other.vPosition), vScale(other.vScale), vRotation(other.vRotation),
		matModel(other.matModel), quat(other.quat),
		parent(other.parent), childs(other.childs),
		bUpdateMatrix(other.bUpdateMatrix)
	{
	}

	SH_GAME_API Transform::Transform(Transform&& other) noexcept:
		Component(std::move(other)),
		position(vPosition), scale(vScale), rotation(vRotation), localToWorldMatrix(matModel),

		worldPosition(other.worldPosition), worldRotation(other.worldRotation), worldScale(other.worldScale),
		vPosition(std::move(other.vPosition)), vScale(std::move(other.vScale)), vRotation(std::move(other.vRotation)),
		matModel(std::move(other.matModel)), quat(std::move(other.quat)),
		parent(other.parent), childs(std::move(other.childs)),
		bUpdateMatrix(other.bUpdateMatrix)
	{
		other.parent = nullptr;
	}

	SH_GAME_API void Transform::OnDestroy()
	{
		if (core::IsValid(parent))
			parent->RemoveChild(*this);
		for (Transform* child : childs)
		{
			if(core::IsValid(child))
				child->gameObject.Destroy();
		}
	}

	SH_GAME_API void Transform::Awake()
	{
		Super::Awake();
	}

	SH_GAME_API void Transform::Start()
	{
	}

	SH_GAME_API void Transform::Update()
	{
		if (bUpdateMatrix)
		{
			UpdateMatrix();
		}
	}

	void Transform::UpdateMatrix()
	{
		if (vRotation.x >= 360)
			vRotation.x -= 360;
		if (vRotation.y >= 360)
			vRotation.y -= 360;
		if (vRotation.z >= 360)
			vRotation.z -= 360;

		matModel = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ vPosition }) * glm::mat4_cast(quat) * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ vScale });
		if (parent)
		{
			matModel = parent->matModel * matModel;
			worldPosition = matModel[3];
			worldQuat = parent->worldQuat * quat;
			worldScale = parent->worldScale * vScale;
#if SH_EDITOR
			worldRotation = glm::degrees(glm::eulerAngles(worldQuat));
#endif
		}
		else
		{
			worldPosition = vPosition;
			worldQuat = quat;
			worldScale = vScale;
			worldRotation = vRotation;
		}

		for (auto it = childs.begin(); it != childs.end();)
		{
			Transform* child = *it;
			if (core::IsValid(child))
				child->UpdateMatrix();
			else
				it = childs.erase(it);
			++it;
		}
		bUpdateMatrix = false;
	}

	void Transform::RemoveChild(const Transform& child)
	{
		auto it = std::find(childs.begin(), childs.end(), &child);
		if (it == childs.end())
			return;
		childs.erase(it);
	}

	SH_GAME_API auto Transform::GetParent() const -> Transform*
	{
		return parent;
	}
	SH_GAME_API auto Transform::GetChildren() const -> const core::SVector<Transform*>&
	{
		return childs;
	}
	SH_GAME_API void Transform::ReorderChildAbove(Transform* child)
	{
		auto it = std::find(childs.begin(), childs.end(), child);
		if (it == childs.end() || it == childs.begin())
			return;

		uint32_t idx = it - childs.begin();
		std::swap(childs[idx], childs[idx - 1]);
	}

	SH_GAME_API void Transform::SetPosition(const Vec3& pos)
	{
		vPosition = pos;
		bUpdateMatrix = true;
	}
	SH_GAME_API void Transform::SetPosition(float x, float y, float z)
	{
		vPosition.x = x;
		vPosition.y = y;
		vPosition.z = z;
		bUpdateMatrix = true;
	}
	SH_GAME_API void Transform::SetScale(const Vec3& scale)
	{
		vScale = scale;
		bUpdateMatrix = true;
	}
	SH_GAME_API void Transform::SetScale(float x, float y, float z)
	{
		vScale.x = x;
		vScale.y = y;
		vScale.z = z;
		bUpdateMatrix = true;
	}
	SH_GAME_API void Transform::SetScale(float scale)
	{
		vScale.x = scale;
		vScale.y = scale;
		vScale.z = scale;
		bUpdateMatrix = true;
	}

	SH_GAME_API void Transform::SetRotation(const Vec3& rot)
	{
		vRotation = rot;

		if (vRotation.x >= 360)
			vRotation.x -= 360;
		if (vRotation.y >= 360)
			vRotation.y -= 360;
		if (vRotation.z >= 360)
			vRotation.z -= 360;

		quat = glm::quat{ glm::radians(glm::vec3{ rot }) };
		bUpdateMatrix = true;
	}
	SH_GAME_API void Transform::SetRotation(const glm::quat& rot)
	{
#if SH_EDITOR
		vRotation = glm::degrees(glm::eulerAngles(quat));
#endif
		quat = rot;
		bUpdateMatrix = true;
	}

	SH_GAME_API void Transform::SetParent(Transform* parent)
	{
		if (parent == this)
			return;

		if (parent && parent->HasChild(*this))
			return;

		if (core::IsValid(parent))
			parent->UpdateMatrix();
		UpdateMatrix();

		if (this->parent) // 이미 다른 부모가 있던 경우
		{
			this->parent->RemoveChild(*this);
			if (core::IsValid(parent))
			{
				this->parent = parent;
				this->parent->childs.push_back(this);

				vPosition = parent->GetWorldToLocalMatrix() * glm::vec4{ glm::vec3{ worldPosition }, 1.f };
				quat = glm::inverse(parent->worldQuat) * worldQuat;
				vScale = (1.f / parent->worldScale) * worldScale;
#if SH_EDITOR
				vRotation = glm::degrees(glm::eulerAngles(glm::inverse(parent->worldQuat) * worldQuat));
#endif
			}
			else
			{
				this->parent = nullptr;
				vPosition = worldPosition;
				quat = glm::quat{ glm::radians(glm::vec3{ worldRotation }) };
				vRotation = worldRotation;
				vScale = worldScale;
				
			}
		}
		else
		{
			if (core::IsValid(parent))
			{
				this->parent = parent;
				this->parent->childs.push_back(this);
				vPosition = parent->GetWorldToLocalMatrix() * glm::vec4{ glm::vec3{ worldPosition }, 1.f };
				quat = glm::inverse(parent->worldQuat) * quat;
				vScale = (1.f / glm::vec3{ parent->worldScale }) * glm::vec3{ worldScale };
#if SH_EDITOR
				vRotation = glm::degrees(glm::eulerAngles(glm::inverse(parent->worldQuat) * quat));
#endif
			}
			else
				return;
		}
		UpdateMatrix();
	}
	SH_GAME_API bool Transform::HasChild(const Transform& child) const
	{
		auto it = std::find(childs.begin(), childs.end(), &child);
		return it != childs.end();
	}

	SH_GAME_API auto Transform::GetQuat() const -> const glm::quat&
	{
		return quat;
	}
	SH_GAME_API auto Transform::GetWorldQuat() const -> const glm::quat&
	{
		return worldQuat;
	}
	SH_GAME_API auto Transform::GetWorldPosition() const -> const Vec3&
	{
		return worldPosition;
	}
	SH_GAME_API auto Transform::GetWorldRotation() const -> const Vec3&
	{
		return worldRotation;
	}
	SH_GAME_API auto Transform::GetWorldScale() const -> const Vec3&
	{
		return worldScale;
	}
	SH_GAME_API auto Transform::GetWorldToLocalMatrix() const -> glm::mat4
	{
		return glm::inverse(matModel);
	}

#if SH_EDITOR
	void Transform::OnPropertyChanged(const core::reflection::Property& property)
	{
		if (std::strcmp(property.GetName(), "vRotation") == 0)
		{
			quat = glm::quat{ glm::radians(glm::vec3{ vRotation }) };
		}
		bUpdateMatrix = true;
	}
#endif
}