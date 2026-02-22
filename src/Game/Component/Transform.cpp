#include "Component/Transform.h"
#include "GameObject.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/mat4x4.hpp"
#include "glm/gtx/orthonormalize.hpp"
#include "glm/gtx/norm.hpp"

#include <algorithm>
#include <cmath>
namespace sh::game
{
	SH_GAME_API Transform::Transform(GameObject& owner) :
		Component(owner),
		position(vPosition), scale(vScale), rotation(vRotation), localToWorldMatrix(matModel),

		worldPosition{ 0.f, 0.f, 0.f }, worldRotation{ 0.f, 0.f, 0.f }, worldScale{ 1.f, 1.f, 1.f },
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

	SH_GAME_API Transform::Transform(Transform&& other) noexcept :
		Component(std::move(other)),
		position(vPosition), scale(vScale), rotation(vRotation), localToWorldMatrix(matModel),

		worldPosition(other.worldPosition), worldRotation(other.worldRotation), worldScale(other.worldScale),
		vPosition(other.vPosition), vScale(other.vScale), vRotation(other.vRotation),
		matModel(other.matModel), quat(other.quat), worldQuat(other.worldQuat),
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
	SH_GAME_API void Transform::BeginUpdate()
	{
		if (bUpdateMatrix)
		{
			UpdateMatrix();
		}
	}
	SH_GAME_API auto Transform::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();
		core::Json& transformJson = mainJson["Transform"];
		transformJson["quat"] = { quat.x, quat.y, quat.z, quat.w };
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

		UpdateMatrix();
	}
	SH_GAME_API void Transform::OnPropertyChanged(const core::reflection::Property& property)
	{
		Super::OnPropertyChanged(property);
		if (IsEditor())
		{
			if (property.GetName() == core::Util::ConstexprHash("vRotation"))
			{
				quat = glm::quat{ glm::radians(glm::vec3{ vRotation }) };
			}
		}
		bUpdateMatrix = true;
	}

	SH_GAME_API void Transform::UpdateMatrix()
	{
		if (IsEditor())
		{
			vRotation.x = std::fmod(vRotation.x, 360.f);
			vRotation.y = std::fmod(vRotation.y, 360.f);
			vRotation.z = std::fmod(vRotation.z, 360.f);
		}

		const glm::mat4 t = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ vPosition });
		const glm::mat4 tInv = glm::translate(glm::mat4{ 1.0f }, -glm::vec3{ vPosition });
		const glm::mat4 r = glm::mat4_cast(quat);
		const glm::mat4 rInv = glm::transpose(r);
		const glm::mat4 s = glm::scale(glm::mat4{ 1.0f }, glm::vec3{ vScale });
		matModel = t * r * s;
		if (vScale.x < 1e-6f || vScale.y < 1e-6f || vScale.z < 1e-6f)
			matModelInv = rInv * tInv;
		else
		{
			const glm::mat4 sInv = glm::scale(glm::mat4{ 1.0f }, glm::vec3{ 1.0f / vScale });
			matModelInv = sInv * rInv * tInv;
		}

		if (parent != nullptr)
		{
			matModel = parent->matModel * matModel;
			matModelInv = matModelInv * parent->matModelInv;

			worldPosition = glm::vec3(matModel[3]);
			worldQuat = parent->worldQuat * quat;
			worldScale = parent->worldScale * vScale;
			if (IsEditor())
				worldRotation = glm::degrees(glm::eulerAngles(worldQuat));
		}
		else
		{
			worldPosition = vPosition;
			worldQuat = quat;
			worldScale = vScale;
			if (IsEditor())
				worldRotation = vRotation;
		}

		// 두포인터를 이용한 함수를 호출하면서 유효하지 않은 자식은 삭제
		std::size_t p0 = 0;
		std::size_t p1 = 0;
		while (p1 < childs.size())
		{
			Transform* const child = childs[p1];
			if (!core::IsValid(child))
			{
				++p1;
				continue;
			}
			if (p0 != p1)
				childs[p0] = childs[p1];
			child->UpdateMatrix();
			++p0;
			++p1;
		}
		if (p0 != p1)
			childs.erase(childs.begin() + p0, childs.end());

		onMatrixUpdate.Notify(matModel);
		bUpdateMatrix = false;
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
			vRotation.x = std::fmod(vRotation.x, 360.f);
		if (vRotation.y >= 360)
			vRotation.y = std::fmod(vRotation.y, 360.f);
		if (vRotation.z >= 360)
			vRotation.z = std::fmod(vRotation.z, 360.f);

		quat = glm::quat{ glm::radians(glm::vec3{ rot }) };
		bUpdateMatrix = true;
	}
	SH_GAME_API void Transform::SetRotation(const glm::quat& rot)
	{
		const float len2 = glm::length2(rot);
		if (std::abs(len2 - 1.0f) < 0.001f)
			this->quat = rot;
		else
			this->quat = glm::normalize(rot);

		if (IsEditor())
			vRotation = glm::degrees(glm::eulerAngles(quat));
		bUpdateMatrix = true;
	}

	SH_GAME_API void Transform::SetQuaternion(const glm::quat& quat)
	{
		const float len2 = glm::length2(quat);
		if (std::abs(len2 - 1.0f) < 0.001f)
			this->quat = quat;
		else
			this->quat = glm::normalize(quat);
		
		if (IsEditor())
			vRotation = glm::degrees(glm::eulerAngles(this->quat));
		bUpdateMatrix = true;
	}

	SH_GAME_API void Transform::SetQuaternion(float x, float y, float z, float w)
	{
		quat = glm::quat{ w, x, y, z };
		const float len2 = glm::length2(quat);
		if (std::abs(len2 - 1.0f) >= 0.001f)
			quat = glm::normalize(quat);

		if (IsEditor())
			vRotation = glm::degrees(glm::eulerAngles(this->quat));
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

	SH_GAME_API void Transform::SetParent(Transform* newParent, bool keepWorld)
	{
		if (newParent == this)
			return;

		if (!core::IsValid(newParent))
			newParent = nullptr;

		if (parent == newParent)
			return;

		if (newParent != nullptr && this->IsAncestorOf(*newParent))
			return;

		if (parent != nullptr && parent->bUpdateMatrix)
			parent->UpdateMatrix();
		if (newParent != nullptr && newParent->bUpdateMatrix)
			newParent->UpdateMatrix();
		if (bUpdateMatrix)
			UpdateMatrix();

		const glm::vec3 oldWorldPos = glm::vec3(worldPosition);
		const glm::quat oldWorldQuat = worldQuat;
		const glm::vec3 oldWorldScale = glm::vec3(worldScale);

		if (parent != nullptr)
			parent->RemoveChild(*this);

		parent = newParent;
		if (parent != nullptr)
			parent->childs.push_back(this);

		if (keepWorld)
		{
			if (parent != nullptr)
			{
				// 내 월드 좌표를 부모의 로컬 좌표계로 변환
				const glm::mat4 parentWorldToLocal = parent->GetWorldToLocalMatrix();
				const glm::vec4 localPos4 = parentWorldToLocal * glm::vec4(oldWorldPos, 1.0f);
				vPosition = glm::vec3(localPos4);

				quat = glm::normalize(glm::inverse(parent->worldQuat) * oldWorldQuat);

				const auto safeDivfn =
					[](const glm::vec3& a, const glm::vec3& b) -> glm::vec3
					{
						constexpr float eps = 1e-8f;
						return glm::vec3{
							(std::abs(b.x) < eps) ? 0.0f : a.x / b.x,
							(std::abs(b.y) < eps) ? 0.0f : a.y / b.y,
							(std::abs(b.z) < eps) ? 0.0f : a.z / b.z
						};
					};
				vScale = safeDivfn(oldWorldScale, glm::vec3(parent->worldScale));
			}
			else
			{
				vPosition = oldWorldPos;
				quat = glm::normalize(oldWorldQuat);
				vScale = oldWorldScale;
			}

			if (IsEditor())
				vRotation = glm::degrees(glm::eulerAngles(quat));
		}
		else
		{
			if (IsEditor())
				vRotation = glm::degrees(glm::eulerAngles(quat));
		}
		bUpdateMatrix = true;
		UpdateMatrix();
	}
	SH_GAME_API bool Transform::HasChild(const Transform& child) const
	{
		auto it = std::find(childs.begin(), childs.end(), &child);
		return it != childs.end();
	}

	SH_GAME_API auto Transform::IsAncestorOf(const Transform& transform) const -> bool
	{
		const Transform* parent = &transform;
		while (parent != nullptr)
		{
			if (parent == this)
				return true;
			parent = parent->parent;
		}
		return false;
	}

	void Transform::RemoveChild(const Transform& child)
	{
		auto it = std::find(childs.begin(), childs.end(), &child);
		if (it == childs.end())
			return;
		childs.erase(it);
	}
}