#include "PCH.h"
#include "Component/Transform.h"

#include "glm/mat4x4.hpp"

namespace sh::game
{
	Transform::Transform() :
		position(vPosition), scale(vScale), rotation(vRot), localToWorldMatrix(matModel),

		localPosition(),
		vPosition(glm::vec3(0.f, 0.f, 0.f)), vScale(glm::vec3(1.0f, 1.0f, 1.0f)), vRot(glm::vec3(0.f, 0.f, 0.f)),
		matModel(), quat(glm::radians(vRot)),
		bUpdateMatrix(false)
	{
		matModel = glm::translate(glm::mat4{1.0f}, vPosition) * glm::mat4_cast(quat) * glm::scale(glm::mat4{ 1.0f }, vScale);
	}
	Transform::~Transform()
	{

	}

	Transform::Transform(const Transform& other) :
		Component(other),
		position(vPosition), scale(vScale), rotation(vRot), localToWorldMatrix(matModel),

		localPosition(other.localPosition),
		vPosition(other.vPosition), vScale(other.vScale), vRot(other.vRot),
		matModel(other.matModel), quat(other.quat),
		bUpdateMatrix(other.bUpdateMatrix)
	{
	}

	Transform::Transform(Transform&& other) noexcept:
		Component(std::move(other)),
		position(vPosition), scale(vScale), rotation(vRot), localToWorldMatrix(matModel),

		localPosition(std::move(other.localPosition)),
		vPosition(std::move(other.vPosition)), vScale(std::move(other.vScale)), vRot(std::move(other.vRot)),
		matModel(std::move(other.matModel)), quat(std::move(other.quat)),
		bUpdateMatrix(other.bUpdateMatrix)
	{
	}

	void Transform::Awake()
	{
		Super::Awake();
	}

	void Transform::Start()
	{
	}

	void Transform::Update()
	{
		if (bUpdateMatrix)
		{
			UpdateMatrix();
			bUpdateMatrix = false;
		}
	}

	void Transform::UpdateMatrix()
	{
		if (vRot.x >= 360)
			vRot.x -= 360;
		if (vRot.y >= 360)
			vRot.y -= 360;
		if (vRot.z >= 360)
			vRot.z -= 360;
		quat = glm::quat{ glm::radians(vRot) };
		matModel = glm::translate(glm::mat4{ 1.0f }, vPosition) * glm::mat4_cast(quat) * glm::scale(glm::mat4{ 1.0f }, vScale);
	}

	void Transform::SetPosition(const glm::vec3& pos)
	{
		vPosition = pos;
		bUpdateMatrix = true;
	}
	void Transform::SetPosition(float x, float y, float z)
	{
		vPosition.x = x;
		vPosition.y = y;
		vPosition.z = z;
		bUpdateMatrix = true;
	}
	void Transform::SetScale(const glm::vec3& scale)
	{
		vScale = scale;
		bUpdateMatrix = true;
	}
	void Transform::SetRotation(const glm::vec3& rot)
	{
		vRot = rot;
		if (vRot.x >= 360)
			vRot.x -= 360;
		if (vRot.y >= 360)
			vRot.y -= 360;
		if (vRot.z >= 360)
			vRot.z -= 360;
		quat = glm::quat{ glm::radians(vRot) };
		bUpdateMatrix = true;
	}
	void Transform::SetRotation(const glm::quat& rot)
	{
		vRot = glm::degrees(glm::eulerAngles(quat));
		quat = rot;
		bUpdateMatrix = true;
	}

#if SH_EDITOR
	void Transform::OnPropertyChanged(const core::reflection::Property& property)
	{
		bUpdateMatrix = true;
	}
#endif
}