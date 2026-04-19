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

	SH_GAME_API void SkinnedMeshRenderer::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == core::Util::ConstexprHash("bones"))
			CalculateIBM();
	}

	SH_GAME_API void SkinnedMeshRenderer::SetSkinnedMesh(render::SkinnedMesh* mesh)
	{
		finalBoneMatrices.fill(glm::mat4{ 1.0f });
		SetMesh(mesh);
	}
	SH_GAME_API void SkinnedMeshRenderer::SetBones(std::vector<Transform*> boneTransforms)
	{
		bones = std::move(boneTransforms);

		CalculateIBM();
	}

	SH_GAME_API void SkinnedMeshRenderer::UpdateDrawable()
	{
		ComputeBoneMatrices();
		UploadBoneMatrices();
		MeshRenderer::UpdateDrawable();
	}

	void SkinnedMeshRenderer::CalculateIBM()
	{
		inverseBindMatrices.resize(bones.size());
		for (std::size_t i = 0; i < bones.size(); ++i)
		{
			if (core::IsValid(bones[i]))
				inverseBindMatrices[i] = glm::inverse(bones[i]->localToWorldMatrix);
			else
				inverseBindMatrices[i] = glm::mat4{ 1.0f };
		}
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
		if (drawables.empty())
			return;

		const std::size_t dataSize = render::SkinnedMesh::MAX_BONES * sizeof(glm::mat4);

		for (std::size_t i = 0; i < drawables.size(); ++i)
		{
			render::Drawable* const drawable = drawables[i];
			if (drawable == nullptr)
				continue;
			const render::Material* const mat = GetMaterial(i);
			if (mat == nullptr || !mat->GetShader())
				continue;

			for (const render::Shader::LightingPassData& lightingPass : mat->GetShader()->GetAllShaderPass())
			{
				for (render::ShaderPass& pass : lightingPass.passes)
				{
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
	}
}//namespace
