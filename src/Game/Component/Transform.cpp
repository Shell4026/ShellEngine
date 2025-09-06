#include "Component/Transform.h"
#include "GameObject.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/mat4x4.hpp"
#include "glm/gtx/orthonormalize.hpp"
#include <algorithm>

namespace sh::game
{
	SH_GAME_API Transform::Transform(GameObject& owner) :
		Component(owner),
		position(vPosition), scale(vScale), rotation(vRotation), localToWorldMatrix(matModel),

		worldPosition(), worldRotation(), worldScale(),
		vPosition{ 0.f, 0.f, 0.f }, vScale{ 1.0f, 1.0f, 1.0f }, vRotation{ 0.f, 0.f, 0.f },
		matModel(), quat(glm::radians(glm::vec3{ vRotation })), worldQuat(quat),
		parent(nullptr), childs(),
		bUpdateMatrix(false)
	{
		matModel = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ vPosition }) * glm::mat4_cast(quat) * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ vScale });
		canPlayInEditor = true;
	}
	SH_GAME_API Transform::~Transform()
	{

	}

	SH_GAME_API Transform::Transform(Transform&& other) noexcept:
		Component(std::move(other)),
		position(vPosition), scale(vScale), rotation(vRotation), localToWorldMatrix(matModel),

		worldPosition(other.worldPosition), worldRotation(other.worldRotation), worldScale(other.worldScale),
		vPosition(std::move(other.vPosition)), vScale(std::move(other.vScale)), vRotation(std::move(other.vRotation)),
		matModel(std::move(other.matModel)), quat(std::move(other.quat)),
		parent(other.parent), childs(std::move(other.childs)),
		bUpdateMatrix(other.bUpdateMatrix),

		onMatrixUpdate(std::move(other.onMatrixUpdate))
	{
		other.parent = nullptr;
	}

	SH_GAME_API auto Transform::operator=(const Transform& other) -> Transform&
	{
		SetParent(other.parent);

		vPosition = other.vPosition;
		vScale = other.vScale;
		vRotation = other.vRotation;
		matModel = other.matModel;
		quat = other.quat;
		
		bUpdateMatrix = true;

		for (auto child : other.childs)
		{
			auto& obj = world.DuplicateGameObject(child->gameObject);
			obj.transform->parent = this;

			childs.push_back(obj.transform);
		}

		return *this;
	}

	SH_GAME_API void Transform::OnDestroy()
	{
		for (Transform* child : childs)
		{
			if (core::IsValid(child))
				child->gameObject.Destroy();
		}
		childs.clear();
		if (core::IsValid(parent))
			parent->RemoveChild(*this);

		Super::OnDestroy();
	}

	SH_GAME_API void Transform::Awake()
	{
		Super::Awake();
		UpdateMatrix();
	}

	SH_GAME_API void Transform::Start()
	{
	}

	SH_GAME_API void Transform::BeginUpdate()
	{
		if (bUpdateMatrix)
		{
			UpdateMatrix();
		}
	}

	SH_GAME_API void Transform::UpdateMatrix()
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
			{
				child->UpdateMatrix();
				++it;
			}
			else
				it = childs.erase(it);
		}
		onMatrixUpdate.Notify(matModel);
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
	SH_GAME_API auto Transform::GetChildren() const -> const std::vector<Transform*>&
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

	SH_GAME_API void Transform::SetModelMatrix(const glm::mat4& matrix)
	{
		glm::vec3 t = glm::vec3{ matrix[3] };

		glm::mat3 m3 = glm::mat3(matrix);
		float sx = glm::length(m3[0]);
		float sy = glm::length(m3[1]);
		float sz = glm::length(m3[2]);
		glm::vec3 s(sx, sy, sz);

		glm::mat3 rmat;
		rmat[0] = m3[0] / sx;
		rmat[1] = m3[1] / sy;
		rmat[2] = m3[2] / sz;
		rmat = glm::orthonormalize(rmat);
		glm::quat q{ rmat };

		SetPosition(t);
		SetRotation(q);
		SetScale(s);
	}

	SH_GAME_API void Transform::SetWorldPosition(const Vec3& pos)
	{
		SetWorldPosition(pos.x, pos.y, pos.z);
	}

	SH_GAME_API void Transform::SetWorldPosition(float x, float y, float z)
	{
		const glm::vec4 worldPos{ x, y, z, 1.0f };
		if (parent != nullptr)
		{
			glm::vec4 localPos = parent->GetWorldToLocalMatrix() * worldPos;
			vPosition = glm::vec3(localPos);
		}
		else
			vPosition = Vec3{ x, y, z };
		bUpdateMatrix = true;
	}

	SH_GAME_API void Transform::SetWorldRotation(const Vec3& rot)
	{
		const glm::quat worldQuat = glm::quat{ glm::radians(glm::vec3{ rot }) };
		SetWorldRotation(worldQuat);
	}

	SH_GAME_API void Transform::SetWorldRotation(const glm::quat& rot)
	{
		if (parent != nullptr)
		{
			glm::quat localQ = glm::inverse(parent->worldQuat) * rot;
			SetRotation(localQ);
		}
		else
		{
			SetRotation(rot);
		}
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

	SH_GAME_API auto Transform::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();
		core::Json& transformJson = mainJson["Transform"];
		transformJson["quat"] = {quat.x, quat.y, quat.z, quat.w};
		if (parent)
			transformJson["parent"] = parent->GetUUID().ToString();

		return mainJson;
	}
	SH_GAME_API void Transform::Deserialize(const core::Json& json)
	{
		const core::Json& transformJson = json["Transform"];
		if (transformJson.contains("parent"))
		{
			std::string uuid = transformJson["parent"].get<std::string>();
			SetParent(static_cast<Transform*>(core::SObjectManager::GetInstance()->GetSObject(core::UUID{ uuid })));
		}

		Super::Deserialize(json);
		
		if (transformJson.contains("quat") && transformJson["quat"].is_array() && transformJson["quat"].size() == 4)
		{
			quat.x = json["Transform"]["quat"][0].get<float>();
			quat.y = json["Transform"]["quat"][1].get<float>();
			quat.z = json["Transform"]["quat"][2].get<float>();
			quat.w = json["Transform"]["quat"][3].get<float>();
		}

		bUpdateMatrix = true;
	}

	void Transform::OnPropertyChanged(const core::reflection::Property& property)
	{
#if SH_EDITOR
		if (property.GetName() == "vRotation")
		{
			quat = glm::quat{ glm::radians(glm::vec3{ vRotation }) };
		}
#endif
		bUpdateMatrix = true;
	}
}