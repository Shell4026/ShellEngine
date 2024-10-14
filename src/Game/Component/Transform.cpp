#include "PCH.h"
#include "Component/Transform.h"
#include "GameObject.h"

#include "glm/mat4x4.hpp"
#include <algorithm>

namespace sh::game
{
	SH_GAME_API Transform::Transform() :
		position(vPosition), scale(vScale), rotation(vRotation), localToWorldMatrix(matModel),

		worldPosition(), worldRotation(), worldScale(),
		vPosition(glm::vec3(0.f, 0.f, 0.f)), vScale(glm::vec3(1.0f, 1.0f, 1.0f)), vRotation(glm::vec3(0.f, 0.f, 0.f)),
		matModel(), quat(glm::radians(vRotation)),
		parent(nullptr), childs(),
		bUpdateMatrix(false)
	{
		matModel = glm::translate(glm::mat4{1.0f}, vPosition) * glm::mat4_cast(quat) * glm::scale(glm::mat4{ 1.0f }, vScale);
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
		if (parent)
			parent->RemoveChild(*this);
		for (Transform* child : childs)
		{
			child->gameObject->Destroy();
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
		quat = glm::quat{ glm::radians(vRotation) };

		matModel = glm::translate(glm::mat4{ 1.0f }, vPosition) * glm::mat4_cast(quat) * glm::scale(glm::mat4{ 1.0f }, vScale);
		if (parent)
		{
			matModel = parent->matModel * matModel;
			worldPosition = glm::vec3(matModel[3]);
			worldRotation = glm::degrees(glm::eulerAngles(parent->quat * quat));
			worldScale = parent->scale * vScale;
		}
		else
		{
			worldPosition = vPosition;
			worldRotation = vRotation;
			worldScale = vScale;
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

	SH_GAME_API auto Transform::GetParent() const -> Transform*
	{
		return parent;
	}
	SH_GAME_API auto Transform::GetChildren() const -> const core::SSet<Transform*>&
	{
		return childs;
	}

	SH_GAME_API void Transform::SetPosition(const glm::vec3& pos)
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
	SH_GAME_API void Transform::SetScale(const glm::vec3& scale)
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

	SH_GAME_API void Transform::SetRotation(const glm::vec3& rot)
	{
		vRotation = rot;
		if (vRotation.x >= 360)
			vRotation.x -= 360;
		if (vRotation.y >= 360)
			vRotation.y -= 360;
		if (vRotation.z >= 360)
			vRotation.z -= 360;
		quat = glm::quat{ glm::radians(vRotation) };
		bUpdateMatrix = true;
	}
	SH_GAME_API void Transform::SetRotation(const glm::quat& rot)
	{
		vRotation = glm::degrees(glm::eulerAngles(quat));
		quat = rot;
		bUpdateMatrix = true;
	}

	SH_GAME_API void Transform::SetParent(Transform* parent)
	{
		if (parent == this)
			return;
		if (core::IsValid(parent))
			parent->UpdateMatrix();
		UpdateMatrix();
		if (this->parent)
		{
			this->parent->RemoveChild(*this);
			if (core::IsValid(parent))
			{
				this->parent = parent;
				this->parent->AddChild(*this);

				vPosition = parent->GetWorldToLocalMatrix() * glm::vec4(worldPosition, 1);
				vRotation = glm::degrees(glm::eulerAngles(glm::inverse(parent->quat) * quat));
				vScale = (1.f / parent->vScale) * scale;
			}
			else
			{
				this->parent = nullptr;
				vPosition = worldPosition;
				vRotation = worldRotation;
				vScale = worldScale;
			}
		}
		else
		{
			if (core::IsValid(parent))
			{
				this->parent = parent;
				this->parent->AddChild(*this);
				vPosition = parent->GetWorldToLocalMatrix() * glm::vec4(worldPosition, 1);
				vRotation = glm::degrees(glm::eulerAngles(glm::inverse(parent->quat) * quat));
				vScale = (1.f / parent->vScale) * scale;
			}
			else
				return;
		}
		UpdateMatrix();
	}
	SH_GAME_API void Transform::AddChild(Transform& child)
	{
		if (!core::IsValid(&child))
			return;
		childs.insert(&child);
	}
	SH_GAME_API void Transform::RemoveChild(Transform& child)
	{
		if (!core::IsValid(&child))
			return;
		auto it = childs.find(&child);
		childs.erase(it);
	}

	SH_GAME_API auto Transform::GetRotationQuat() const -> const glm::quat&
	{
		return quat;
	}
	SH_GAME_API auto Transform::GetWorldPosition() const -> const glm::vec3&
	{
		return worldPosition;
	}
	SH_GAME_API auto Transform::GetWorldRotation() const -> const glm::vec3&
	{
		return worldRotation;
	}
	SH_GAME_API auto Transform::GetWorldScale() const -> const glm::vec3&
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
		bUpdateMatrix = true;
	}
#endif
}