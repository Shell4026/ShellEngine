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
		Super::Awake();
		const render::SkinnedMesh* const skinnedMesh = static_cast<const render::SkinnedMesh*>(GetMesh());
		if (core::IsValid(skinnedMesh))
			inverseBindMatrices = skinnedMesh->GetInverseBindMatrices();
	}

	SH_GAME_API void SkinnedMeshRenderer::SetSkinnedMesh(render::SkinnedMesh* mesh)
	{
		finalBoneMatrices.fill(glm::mat4{ 1.0f });
		SetMesh(mesh);
	}
	SH_GAME_API void SkinnedMeshRenderer::SetBones(std::vector<Transform*> boneTransforms)
	{
		bones = std::move(boneTransforms);

		const render::SkinnedMesh* const skinnedMesh = static_cast<const render::SkinnedMesh*>(GetMesh());
		if (core::IsValid(skinnedMesh))
			inverseBindMatrices = skinnedMesh->GetInverseBindMatrices();
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
						pass.GetSkinBinding(),
						finalBoneMatrices.data(),
						dataSize);
				}
			}
		}
	}
}//namespace
