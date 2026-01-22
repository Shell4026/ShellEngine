#include "EditorResource.h"
#include "UI/Project.h"

#include "Core/AssetExporter.h"

#include "Render/VulkanImpl/VulkanContext.h"
#include "Render/VulkanImpl/VulkanShaderPassBuilder.h"
#include "Render/Material.h"
#include "Render/RenderTexture.h"

#include "Game/Asset/TextureLoader.h"
#include "Game/Asset/ShaderLoader.h"
#include "Game/Asset/ModelLoader.h"
#include "Game/Asset/ShaderAsset.h"
#include "Game/Asset/TextureAsset.h"

#include <filesystem>
namespace sh::editor
{
	void EditorResource::ExtractAllAssetToLibrary(Project& project)
	{
		for (auto& [name, shaderPtr] : shaders)
		{
			if (!core::IsValid(shaderPtr))
				continue;

			game::ShaderAsset asset{ *shaderPtr };
			auto writeTime = std::filesystem::file_time_type::clock::now();
			asset.SetWriteTime(writeTime.time_since_epoch().count());

			const auto path = project.GetLibraryPath() / (shaderPtr->GetUUID().ToString() + ".asset");
			if (core::AssetExporter::Save(asset, path, true))
			{
				std::filesystem::last_write_time(path, writeTime);
			}
		}
	}
	SH_EDITOR_API void EditorResource::LoadAllAssets(Project& project)
	{
		auto& ctx = *project.renderer.GetContext();
		auto& renderer = project.renderer;

		render::vk::VulkanShaderPassBuilder shaderBuilder{ static_cast<sh::render::vk::VulkanContext&>(*project.renderer.GetContext()) }; // TODO

		game::TextureLoader texLoader{ ctx };
		game::ModelLoader modelLoader{ *renderer.GetContext() };
		game::ShaderLoader shaderLoader{ &shaderBuilder };

		auto folderTex = static_cast<render::Texture*>(project.loadedAssets.AddResource(core::UUID::Generate(), static_cast<render::Texture*>(texLoader.Load("textures/folder.png"))));
		assert(folderTex != nullptr);
		auto fileTex = static_cast<render::Texture*>(project.loadedAssets.AddResource(core::UUID::Generate(), static_cast<render::Texture*>(texLoader.Load("textures/file.png"))));
		assert(fileTex != nullptr);
		auto meshTex = static_cast<render::Texture*>(project.loadedAssets.AddResource(core::UUID::Generate(), static_cast<render::Texture*>(texLoader.Load("textures/meshIcon.png"))));
		assert(meshTex != nullptr);
		auto materialTex = static_cast<render::Texture*>(project.loadedAssets.AddResource(core::UUID::Generate(), static_cast<render::Texture*>(texLoader.Load("textures/MaterialIcon.png"))));
		assert(materialTex != nullptr);

		folderIcon.Create(ctx, *folderTex);
		fileIcon.Create(ctx, *fileTex);
		meshIcon.Create(ctx, *meshTex);
		materialIcon.Create(ctx, *materialTex);

		auto lineShader = shaders.insert_or_assign("Line", static_cast<render::Shader*>(shaderLoader.Load("shaders/line.shader"))).first->second;
		auto errorShader = shaders.insert_or_assign("ErrorShader", static_cast<render::Shader*>(shaderLoader.Load("shaders/error.shader"))).first->second;
		auto gridShader = shaders.insert_or_assign("GridShader", static_cast<render::Shader*>(shaderLoader.Load("shaders/grid.shader"))).first->second;
		auto pickingShader = shaders.insert_or_assign("EditorPickingShader", static_cast<render::Shader*>(shaderLoader.Load("shaders/EditorPicking.shader"))).first->second;
		auto outlineShader = shaders.insert_or_assign("OutlineShader", static_cast<render::Shader*>(shaderLoader.Load("shaders/outline.shader"))).first->second;
		auto triangleShader = shaders.insert_or_assign("TriangleShader", static_cast<render::Shader*>(shaderLoader.Load("shaders/triangle.shader"))).first->second;
		auto outlinePreShader = shaders.insert_or_assign("OutlinePreShader", static_cast<render::Shader*>(shaderLoader.Load("shaders/EditorOutlinePre.shader"))).first->second;
		auto outlinePostShader = shaders.insert_or_assign("OutlinePostShader", static_cast<render::Shader*>(shaderLoader.Load("shaders/EditorOutlinePost.shader"))).first->second;

		auto blackTex = textures.insert_or_assign("BlackTexture", static_cast<render::Texture*>(texLoader.Load("textures/black.png"))).first->second;
		blackTex->Build(*renderer.GetContext());
		blackTex->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f18" });

		auto errorMat = materials.insert_or_assign("ErrorMaterial", core::SObject::Create<render::Material>(errorShader)).first->second;
		auto lineMat = materials.insert_or_assign("LineMaterial", core::SObject::Create<render::Material>(lineShader)).first->second;
		auto gridMat = materials.insert_or_assign("GridMaterial", core::SObject::Create<render::Material>(gridShader)).first->second;
		auto pickingMat = materials.insert_or_assign("PickingMaterial", core::SObject::Create<render::Material>(pickingShader)).first->second;
		auto triMat = materials.insert_or_assign("TriangleMaterial", core::SObject::Create<render::Material>(triangleShader)).first->second;
		auto outlinePreMat = materials.insert_or_assign("OutlinePreMaterial", core::SObject::Create<render::Material>(outlinePreShader)).first->second;
		auto outlinePostMat = materials.insert_or_assign("OutlinePostMaterial", core::SObject::Create<render::Material>(outlinePostShader)).first->second;

		auto sphereModel = models.insert_or_assign("SphereModel", static_cast<render::Model*>(modelLoader.Load("model/Sphere.obj"))).first->second;
		sphereModel->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f15" });
		sphereModel->GetMeshes()[0]->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f19" });
		auto cubeModel = models.insert_or_assign("CubeModel", static_cast<render::Model*>(modelLoader.Load("model/cube.obj"))).first->second;
		cubeModel->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f16" });
		cubeModel->GetMeshes()[0]->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f20" });
		auto planeModel = models.insert_or_assign("PlaneModel", static_cast<render::Model*>(modelLoader.Load("model/Plane.glb"))).first->second;
		planeModel->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f17" });
		planeModel->GetMeshes()[0]->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f21" });

		render::RenderTargetLayout rt{};
		rt.format = render::TextureFormat::R8;
		rt.depthFormat = render::TextureFormat::D24S8;
		rt.bUseMSAA = false;

		render::RenderTexture* outlineTexture = core::SObject::Create<render::RenderTexture>(rt);
		outlineTexture->SetSize(1024, 1024);
		outlineTexture->Build(*renderer.GetContext());
		textures.insert_or_assign("OutlineTexture", outlineTexture);

		errorShader->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f0f" });

		errorMat->Build(*renderer.GetContext());
		errorMat->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f10" });
		lineMat->SetName("Line");
		lineMat->Build(*renderer.GetContext());
		lineMat->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f11" });
		pickingMat->SetName("PickingMaterial");
		pickingMat->Build(*renderer.GetContext());
		pickingMat->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f12" });

		gridMat->SetProperty("color", glm::vec4{ 0.6f, 0.6f, 0.8f, 0.2f });
		gridMat->Build(*renderer.GetContext());

		triMat->SetProperty("color", glm::vec3{ 0.f, 1.f, 0.f });
		triMat->Build(*renderer.GetContext());

		outlinePreMat->Build(*renderer.GetContext());
		outlinePreMat->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f13" });

		outlinePostMat->SetProperty("outlineWidth", 1.0f);
		outlinePostMat->SetProperty("outlineColor", glm::vec4{ 41 / 255.0f, 74 / 255.0f, 122 / 255.0f, 1.0f });
		outlinePostMat->SetProperty("tex", outlineTexture);
		outlinePostMat->Build(*renderer.GetContext());
		outlinePostMat->SetUUID(core::UUID{ "bbc4ef7ec45dce223297a224f8093f14" });
	}

	SH_EDITOR_API auto EditorResource::GetIcon(Icon icon) const -> const game::GUITexture*
	{
		switch (icon)
		{
		case Icon::Folder:
			return &folderIcon;
		case Icon::File:
			return &fileIcon;
		case Icon::Mesh:
			return &meshIcon;
		case Icon::Material:
			return &materialIcon;
		default:
			return nullptr;
		}
	}

	SH_EDITOR_API auto EditorResource::GetShader(const std::string& name) -> render::Shader*
	{
		auto it = shaders.find(name);
		if (it == shaders.end())
			return nullptr;
		return it->second;
	}

	SH_EDITOR_API auto EditorResource::GetMaterial(const std::string& name) -> render::Material*
	{
		auto it = materials.find(name);
		if (it == materials.end())
			return nullptr;
		return it->second;
	}

	SH_EDITOR_API auto EditorResource::GetTexture(const std::string& name) -> render::Texture*
	{
		auto it = textures.find(name);
		if (it == textures.end())
			return nullptr;
		return it->second;
	}
	SH_EDITOR_API auto EditorResource::GetModel(const std::string& name) -> render::Model*
	{
		auto it = models.find(name);
		if (it == models.end())
			return nullptr;
		return it->second;
	}
}//namespace