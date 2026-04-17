#include "Component/Render/SkinnedMeshRenderer.h"
#include "Game/Component/Transform.h"

#include "Render/UniformStructLayout.h"
#include "Render/Shader.h"

#include <glm/gtc/type_ptr.hpp>

namespace sh::game
{
	SkinnedMeshRenderer::SkinnedMeshRenderer(GameObject& owner) :
		MeshRenderer(owner)
	{
		finalBoneMatrices.fill(glm::mat4{ 1.0f });
	}
	SkinnedMeshRenderer::~SkinnedMeshRenderer() = default;

	SH_GAME_API void SkinnedMeshRenderer::Awake()
	{
		MeshRenderer::Awake();
	}
	SH_GAME_API void SkinnedMeshRenderer::Start()
	{
		MeshRenderer::Start();
	}

	SH_GAME_API auto SkinnedMeshRenderer::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();
		core::Json& skinJson = mainJson["SkinnedMeshRenderer"];
		for (const glm::mat4& ibm : inverseBindMatrices)
		{
			core::Json matJson;
			for (int i = 0; i < 4; ++i)
				matJson.push_back(core::Json{ ibm[i].x, ibm[i].y, ibm[i].z, ibm[i].w });
			skinJson["ibm"].push_back(std::move(matJson));
		}

		return mainJson;
	}

	SH_GAME_API void SkinnedMeshRenderer::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);
		inverseBindMatrices.clear();
		const core::Json& skinJson = json["SkinnedMeshRenderer"];
		if (!skinJson.contains("ibm"))
			return;
		inverseBindMatrices.resize(skinJson["ibm"].size());
		int i = 0;
		for (const core::Json& matJson : skinJson["ibm"])
		{
			std::array<float, 16> arr{ 0.f };
			if (matJson.size() == 4)
			{
				for (int col = 0; col < 4; ++col)
				{
					for (int row = 0; row < 4; ++row)
					{
						arr[col * 4 + row] = matJson[col][row];
					}
				}
				inverseBindMatrices[i] = glm::make_mat4(arr.data());
			}
			++i;
		}
	}

	SH_GAME_API void SkinnedMeshRenderer::SetSkinnedMesh(render::SkinnedMesh* mesh)
	{
		finalBoneMatrices.fill(glm::mat4{ 1.0f });
		SetMesh(mesh);
	}
	SH_GAME_API void SkinnedMeshRenderer::SetBones(std::vector<Transform*> boneTransforms)
	{
		bones = std::move(boneTransforms);

		inverseBindMatrices.resize(bones.size());
		for (std::size_t i = 0; i < bones.size(); ++i)
		{
			if (core::IsValid(bones[i]))
				inverseBindMatrices[i] = glm::inverse(bones[i]->localToWorldMatrix);
			else
				inverseBindMatrices[i] = glm::mat4{ 1.0f };
		}
	}

	SH_GAME_API void SkinnedMeshRenderer::CreateDrawable()
	{
		MeshRenderer::CreateDrawable();
	}
	SH_GAME_API void SkinnedMeshRenderer::UpdateDrawable()
	{
		ComputeBoneMatrices();
		UploadBoneMatrices();
		MeshRenderer::UpdateDrawable();
	}

	void SkinnedMeshRenderer::ComputeBoneMatrices()
	{
		const std::size_t jointCount = std::min(bones.size(), inverseBindMatrices.size());
		if (jointCount == 0)
			return;

		for (std::size_t i = 0; i < jointCount; ++i)
		{
			if (!core::IsValid(bones[i]))
				continue;
			// finalBoneMatrix[i] = bone 월드 행렬 * 역바인드 행렬
			finalBoneMatrices[i] = bones[i]->localToWorldMatrix * inverseBindMatrices[i];
		}
	}
	void SkinnedMeshRenderer::UploadBoneMatrices()
	{
		if (!drawable)
			return;

		const render::Material* mat = GetMaterial();
		if (!mat || !mat->GetShader())
			return;

		const std::size_t dataSize = render::SkinnedMesh::MAX_BONES * sizeof(glm::mat4);

		for (const auto& lightingPass : mat->GetShader()->GetAllShaderPass())
		{
			for (const auto& passRef : lightingPass.passes)
			{
				render::ShaderPass& pass = passRef.get();
				if (pass.IsPendingKill())
					continue;
				drawable->GetMaterialData().SetUniformData(
					pass,
					render::UniformStructLayout::Type::Object,
					0,
					finalBoneMatrices.data(),
					dataSize);
			}
		}
	}
}//namespace
