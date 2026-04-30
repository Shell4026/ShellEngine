#include "Component/Render/SkinnedMeshRenderer.h"
#include "Game/Component/Transform.h"

#include "Core/ThreadPool.h"

#include "Render/UniformStructLayout.h"
#include "Render/Shader.h"

#include <glm/gtc/type_ptr.hpp>

namespace sh::game
{
	SkinnedMeshRenderer::SkinnedMeshRenderer(GameObject& owner) :
		MeshRenderer(owner)
	{
	}
	SkinnedMeshRenderer::~SkinnedMeshRenderer() = default;

	SH_GAME_API void SkinnedMeshRenderer::Awake()
	{
		Super::Awake();
		const render::SkinnedMesh* const skinnedMesh = static_cast<const render::SkinnedMesh*>(GetMesh());
		if (core::IsValid(skinnedMesh))
			inverseBindMatrices = skinnedMesh->GetInverseBindMatrices();
	}

	SH_GAME_API void SkinnedMeshRenderer::OnPropertyChanged(const core::reflection::Property& prop)
	{
		Super::OnPropertyChanged(prop);
		if (prop.GetName() == core::Util::ConstexprHash("bones"))
			InitIBM();
	}

	SH_GAME_API void SkinnedMeshRenderer::SetSkinnedMesh(render::SkinnedMesh* mesh)
	{
		SetMesh(mesh);
	}
	SH_GAME_API void SkinnedMeshRenderer::SetBones(std::vector<Transform*> boneTransforms)
	{
		bones = std::move(boneTransforms);
		InitIBM();
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

		if (finalBoneMatrices.size() < bones.size())
			finalBoneMatrices.resize(bones.size());

		const auto calcMatricesFn =
			[this](std::size_t start, std::size_t end)
			{
				for (std::size_t i = start; i < end; ++i)
				{
					if (!core::IsValid(bones[i]))
						continue;
					finalBoneMatrices[i] = bones[i]->localToWorldMatrix * inverseBindMatrices[i];
				}
			};

		if (jointCount > 100)
		{
			static core::ThreadPool& threadPool = *core::ThreadPool::GetInstance();
			const std::size_t threadCount = 4;
			std::vector<std::future<void>> futures(threadCount);
			const std::size_t perTask = jointCount / threadCount;
			const std::size_t rest = jointCount % threadCount;

			std::size_t start = 0;
			std::size_t futureIdx = 0;
			for (int i = 0; i < rest; ++i)
			{
				futures[futureIdx++] = threadPool.AddTask(calcMatricesFn, start, start + perTask + 1);
				start = start + perTask + 1;
			}
			for (int i = 0; i < threadCount - rest; ++i)
			{
				futures[futureIdx++] = threadPool.AddTask(calcMatricesFn, start, start + perTask);
				start = start + perTask;
			}
			for (std::future<void>& future : futures)
				future.get();
		}
		else
			calcMatricesFn(0, jointCount);
	}
	void SkinnedMeshRenderer::UploadBoneMatrices()
	{
		if (drawables.empty())
			return;

		const std::size_t dataSize = bones.size() * sizeof(glm::mat4);

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
					drawable->GetMaterialData().SetBindingData(
						pass,
						render::UniformStructLayout::Usage::Object,
						pass.GetSkinBinding(),
						finalBoneMatrices.data(),
						dataSize);
				}
			}
		}
	}
	void SkinnedMeshRenderer::InitIBM()
	{
		finalBoneMatrices.resize(bones.size());

		const render::SkinnedMesh* const skinnedMesh = static_cast<const render::SkinnedMesh*>(GetMesh());
		if (core::IsValid(skinnedMesh))
			inverseBindMatrices = skinnedMesh->GetInverseBindMatrices();
	}
}//namespace
