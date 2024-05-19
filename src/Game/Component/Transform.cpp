#include "Component/Transform.h"

#include "glm/mat4x4.hpp"

namespace sh::game
{
	Transform::Transform(GameObject& owner) :
		Component(owner),
		position(vPosition), scale(vScale), rotation(vRot), localToWorldMatrix(matModel),

		localPosition(),
		vPosition(glm::vec3(0.f, 0.f, 0.f)), vScale(glm::vec3(1.0f, 1.0f, 1.0f)), vRot(glm::vec3(0.f, 0.f, 0.f)),
		matModel(), quat(vRot),
		updateMatrix(false)
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
		updateMatrix(other.updateMatrix)
	{
	}

	Transform::Transform(Transform&& other) noexcept:
		Component(std::move(other)),
		position(vPosition), scale(vScale), rotation(vRot), localToWorldMatrix(matModel),

		localPosition(std::move(other.localPosition)),
		vPosition(std::move(other.vPosition)), vScale(std::move(other.vScale)), vRot(std::move(other.vRot)),
		matModel(std::move(other.matModel)), quat(std::move(other.quat)),
		updateMatrix(other.updateMatrix)
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
		if (updateMatrix)
		{
			UpdateMatrix();
			updateMatrix = false;
		}
	}

	void Transform::UpdateMatrix()
	{
		matModel = glm::translate(glm::mat4{ 1.0f }, vPosition) * glm::mat4_cast(quat) * glm::scale(glm::mat4{ 1.0f }, vScale);
	}

	void Transform::SetPosition(const glm::vec3& pos)
	{
		vPosition = pos;
		updateMatrix = true;
	}
	void Transform::SetScale(const glm::vec3& scale)
	{
		vScale = scale;
		updateMatrix = true;
	}
	void Transform::SetRotation(const glm::vec3& rot)
	{
		vRot = rot;
		quat = glm::quat{ glm::radians(vRot) };
		updateMatrix = true;
	}
	void Transform::SetRotation(const glm::quat& rot)
	{
		vRot = glm::degrees(glm::eulerAngles(quat));
		quat = rot;
		updateMatrix = true;
	}
}