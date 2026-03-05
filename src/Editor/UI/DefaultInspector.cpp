#include "UI/DefaultInspector.h"
#include "UI/Inspector.h"
#include "AssetDatabase.h"

#include "Game/World.h"

#include "External/imgui/misc/cpp/imgui_stdlib.h"
#include "imgui.h"

#include <cstdint>
#include <algorithm>
namespace sh::editor
{
	SH_EDITOR_API void GameObjectInspector::RenderUI(const std::vector<core::SObject*>& objs, int idx)
	{
		auto mainGameObjPtr = static_cast<game::GameObject*>(objs.back());
		auto& gameObjs = reinterpret_cast<const std::vector<game::GameObject*>&>(objs);
		if (!core::IsValid(mainGameObjPtr))
			return;

		bool bActive = mainGameObjPtr->activeSelf;
		if (ImGui::Checkbox("##active", &bActive))
		{
			for (auto gameObjPtr : gameObjs)
				if (gameObjPtr != nullptr)
					gameObjPtr->SetActive(bActive);
		}
		ImGui::SameLine();
		ImGui::Text("Active");
		
		bool bEditorOnly = mainGameObjPtr->bEditorOnly;
		if (ImGui::Checkbox("##editorOnly", &bEditorOnly))
		{
			for (auto gameObjPtr : gameObjs)
				if (gameObjPtr != nullptr)
					gameObjPtr->bEditorOnly = bEditorOnly;
		}
		ImGui::SameLine();
		ImGui::Text("EditorOnly");

		ImGui::Separator();
		
		Inspector::RenderProperties(mainGameObjPtr->transform->GetType(), *mainGameObjPtr->transform, 0);
		ImGui::Separator();
		ImGui::Text("Components");

		// 드래그 드랍으로 도중에 컴포넌트가 추가 되는 일이 발생한다.
		// 그로인해 반복자가 깨지므로 컴포넌트 배열을 복사 해둬야 한다.
		std::vector<game::Component*> components = mainGameObjPtr->GetComponents();
		int componentsIdx = 1;
		for (auto component : components)
		{
			if (!core::IsValid(component))
				continue;
			if (component->hideInspector)
				continue;
			const std::string componentName = component->GetType().name.ToString();
			bool bOpenComponent = ImGui::CollapsingHeader((componentName.c_str() + ("##" + std::to_string(componentsIdx))).data());
			if (ImGui::BeginPopupContextItem((component->GetUUID().ToString() + "RightClickPopup").c_str()))
			{
				if (component->GetType() != game::Transform::GetStaticType())
				{
					if (ImGui::Selectable("Delete"))
						component->Destroy();
				}
				ImGui::EndPopup();
			}
			if (bOpenComponent && core::IsValid(component))
			{
				std::vector<core::SObject*> components{ component };
				Inspector::RenderPropertiesCustomInspector(component->GetType(), components, componentsIdx);
			}
			++componentsIdx;
		}//for auto& component
		ImGui::Separator();

		RenderAddComponent(gameObjs);
	}
	void GameObjectInspector::RenderAddComponent(const std::vector<game::GameObject*>& gameObjs)
	{
		if (ImGui::Button("Add Component", { -FLT_MIN, 0.0f }))
		{
			componentItems.clear();
			auto& components = gameObjs.back()->world.componentModule.GetComponents();
			for (auto& [fullname, _] : components)
			{
				auto [group, name] = GetComponentGroupAndName(fullname);
				auto it = componentItems.find(group);
				if (it == componentItems.end())
					componentItems.insert({ group, std::vector<std::string>{std::move(name)} });
				else
					it->second.push_back(std::move(name));
			}

			bAddComponent = !bAddComponent;
		}

		if (bAddComponent)
		{
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::BeginChild("ComponentsList", ImVec2(0, 200), ImGuiChildFlags_::ImGuiChildFlags_Border);
			for (auto& [group, vector] : componentItems)
			{
				const char* groupName = group.c_str();
				if (group.empty())
					groupName = "Default";
				if (ImGui::CollapsingHeader(groupName))
				{
					for (auto& name : vector)
					{
						if (ImGui::Selectable(name.c_str()))
						{
							std::string searchName = name;
							if (!group.empty())
								searchName = group + "/" + name;
							for (auto objPtr : gameObjs)
								if (objPtr != nullptr)
									objPtr->AddComponent(objPtr->world.componentModule.GetComponents().at(searchName)->Create(*objPtr));
							bAddComponent = false;
						}
					}
				}
			}
			ImGui::EndChild();
			if (ImGui::Button("Close"))
				bAddComponent = false;
		}
	}
	auto GameObjectInspector::GetComponentGroupAndName(std::string_view fullname) -> std::pair<std::string, std::string>
	{
		auto pos = fullname.find('/');
		std::string group{}, name{};
		if (pos == fullname.npos)
			name = fullname;
		else
		{
			group = fullname.substr(0, pos);
			name = fullname.substr(pos + 1);
		}
		return { std::move(group), std::move(name) };
	}
	SH_EDITOR_API void MaterialInspector::RenderUI(const std::vector<core::SObject*>& objs, int idx)
	{
		ImGui::Separator();
		render::Material* mat = reinterpret_cast<render::Material*>(objs.back());
		const auto& mats = reinterpret_cast<const std::vector<render::Material*>&>(objs);

		render::Shader* const shader = mat->GetShader();
		bool bSameShaders = true;
		bool bChanged = false;
		for (auto matPtr : mats)
		{
			if (matPtr == nullptr)
				continue;
			if (matPtr->GetShader() != shader)
			{
				bSameShaders = false;
				break;
			}
		}

		{
			float iconSize = 20;
			float buttonWidth = ImGui::GetContentRegionAvail().x - iconSize;

			std::string shaderName = "Empty";
			if (core::IsValid(shader))
				shaderName = shader->GetName().ToString();

			ImGui::Text("Shader");
			if (ImGui::Button(shaderName.c_str(), ImVec2{ buttonWidth, iconSize }))
			{
			}

			if (ImGui::BeginDragDropTarget())
			{
				auto p = ImGui::GetCurrentContext()->DragDropPayload;
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(std::string{ core::reflection::TypeTraits::GetTypeName<render::Shader>() }.c_str());
				if (payload)
				{
					render::Shader* shader = *reinterpret_cast<render::Shader**>(payload->Data);
					for (auto matPtr : mats)
						if (matPtr != nullptr)
							matPtr->SetShader(shader);
					bChanged = true;
				}
				ImGui::EndDragDropTarget();
			}
		}

		if (core::IsValid(shader) && bSameShaders)
		{
			for (auto& [name, propInfo] : shader->GetProperties())
			{
				ImGui::Text(name.c_str());
				if (propInfo.type == core::reflection::GetType<float>())
				{
					float parameter = mat->GetProperty<float>(name).value_or(0.f);
					if (ImGui::InputFloat(("##input_" + name).c_str(), &parameter))
					{
						for (auto matPtr : mats)
							if (matPtr != nullptr)
								matPtr->SetProperty(name, parameter);
						bChanged = true;
					}
				}
				else if (propInfo.type == core::reflection::GetType<glm::vec2>())
				{
					glm::vec2 v = mat->GetProperty<glm::vec2>(name).value_or(glm::vec2{ 0.f, 0.f });
					if (ImGui::InputFloat2(("##input_" + name).c_str(), &v[0]))
					{
						for (auto matPtr : mats)
							if (matPtr != nullptr)
								matPtr->SetProperty(name, v);
						bChanged = true;
					}
				}
				else if (propInfo.type == core::reflection::GetType<glm::vec3>())
				{
					glm::vec3 v = mat->GetProperty<glm::vec3>(name).value_or(glm::vec3{ 0.f });
					if (ImGui::InputFloat3(("##input_" + name).c_str(), &v[0]))
					{
						for (auto matPtr : mats)
							if (matPtr != nullptr)
								matPtr->SetProperty(name, v);
						bChanged = true;
					}
				}
				else if (propInfo.type == core::reflection::GetType<glm::vec4>())
				{
					glm::vec4 v = mat->GetProperty<glm::vec4>(name).value_or(glm::vec4{ 0.f });
					if (ImGui::InputFloat4(("##input_" + name).c_str(), &v[0]))
					{
						for (auto matPtr : mats)
							if (matPtr != nullptr)
								matPtr->SetProperty(name, v);
						bChanged = true;
					}
				}
				else if (propInfo.type == core::reflection::GetType<render::Texture>())
				{
					float iconSize = 20;
					float buttonWidth = ImGui::GetContentRegionAvail().x - iconSize;

					auto texProp = mat->GetProperty<const render::Texture*>(name);
					const render::Texture* const tex = texProp.value();
					std::string texName = "Empty";
					if (core::IsValid(tex))
						texName = tex->GetName().ToString();
					if (ImGui::Button(texName.c_str(), ImVec2{buttonWidth, iconSize}))
					{

					}
					if (ImGui::BeginDragDropTarget())
					{
						auto p = ImGui::GetCurrentContext()->DragDropPayload;
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(std::string{ core::reflection::TypeTraits::GetTypeName<render::Texture>() }.c_str());
						if (payload != nullptr)
						{
							const render::Texture* texture = *reinterpret_cast<render::Texture**>(payload->Data);
							for (auto matPtr : mats)
								if (matPtr != nullptr)
									matPtr->SetProperty(name, texture);
							bChanged = true;
						}

						ImGui::EndDragDropTarget();
					}
				}
			}
			if (ImGui::TreeNode("Constant value"))
			{
				for (auto& lightingPass : shader->GetAllShaderPass())
				{
					for (render::ShaderPass& pass : lightingPass.passes)
					{
						const auto& data = mat->GetConstantData(pass);

						for (auto& [name, info] : pass.GetConstants())
						{
							const int* dataPtr = reinterpret_cast<const int*>(data->data() + info.offset);
							int dataValue = *dataPtr;
							ImGui::Text("%s", name.c_str());
							if (ImGui::InputInt(("##constant_" + name).c_str(), &dataValue, 0, 0))
							{
								for (auto matPtr : mats)
									if (matPtr != nullptr)
										matPtr->SetConstant(name, dataValue);
								bChanged = true;
							}
						}

					}
				}
				ImGui::TreePop();
			}
			
			if (bChanged)
			{
				for (auto matPtr : mats)
					if (matPtr != nullptr)
						AssetDatabase::GetInstance()->SetDirty(matPtr);
				AssetDatabase::GetInstance()->SaveAllAssets();
			}
		}
		ImGui::Separator();
	}
	TextureInspector::TextureInspector()
	{
		previewTex = core::SObject::Create<game::GUITexture>();
		previewTex->SetName("preview");
		core::GarbageCollection::GetInstance()->SetRootSet(previewTex);
	}
	TextureInspector::~TextureInspector()
	{
	}
	SH_EDITOR_API void TextureInspector::RenderUI(const std::vector<core::SObject*>& objs, int idx)
	{
		using namespace render;
		Texture* const texture = reinterpret_cast<Texture*>(objs.back());
		auto& textures = reinterpret_cast<const std::vector<Texture*>&>(objs);
		TextureFormat format = texture->GetTextureFormat();

		bool bChanged = false;

		std::string formatText = "";
		switch (format)
		{
		case TextureFormat::RGB24:
			formatText = "RGB24";
			break;
		case TextureFormat::RGBA32:
			formatText = "RGBA32";
			break;
		case TextureFormat::SRGB24:
			formatText = "SRGB24";
			break;
		case TextureFormat::SRGBA32:
			formatText = "SRGBA32";
			break;
		case TextureFormat::R8:
			formatText = "R8";
			break;
		}
		ImGui::Text("format: %s", formatText.c_str());
		ImGui::Text("width: %d", texture->GetWidth());
		ImGui::Text("height: %d", texture->GetHeight());
		ImGui::Text("mipLevel: %d", texture->GetMipLevel());

		ImGui::Separator();

		ImGui::Text("SRGB");
		bool bSRGB = texture->IsSRGB();
		if (ImGui::Checkbox("##SRGB", &bSRGB))
		{
			if (bSRGB)
			{
				if (format == TextureFormat::RGB24)
				{
					for (auto texPtr : textures)
						if (texPtr != nullptr)
							texPtr->ChangeTextureFormat(TextureFormat::SRGB24);
				}
				else if(format == TextureFormat::RGBA32)
				{
					for (auto texPtr : textures)
						if (texPtr != nullptr)
							texPtr->ChangeTextureFormat(TextureFormat::SRGBA32);
				}
			}
			else
			{
				if (format == TextureFormat::SRGB24)
				{
					for (auto texPtr : textures)
						if (texPtr != nullptr)
							texPtr->ChangeTextureFormat(TextureFormat::RGB24);
				}
				else if (format == TextureFormat::SRGBA32)
				{
					for (auto texPtr : textures)
						if (texPtr != nullptr)
							texPtr->ChangeTextureFormat(TextureFormat::RGBA32);
				}
			}
			bChanged = true;
		}
		bool bGenerateMipmap = texture->IsGenerateMipmap();
		ImGui::Text("Generate mipmap");
		if (ImGui::Checkbox("##mipmap", &bGenerateMipmap))
		{
			for (auto texPtr : textures)
				if (texPtr != nullptr)
					texPtr->SetGenerateMipmap(bGenerateMipmap);
			bChanged = true;
		}
		int anisoLevel = texture->GetAnisoLevel();
		ImGui::Text("Aniso");
		if (ImGui::InputInt("##Aniso", &anisoLevel))
		{
			anisoLevel = std::clamp(anisoLevel, 0, 12);
			for (auto texPtr : textures)
				if (texPtr != nullptr)
					texPtr->SetAnisoLevel(anisoLevel);
			bChanged = true;
		}
		ImGui::Text("Filtering");
		{
			const char* filters[] = { "Box", "Linear" };
			static int current = 0;
			current = static_cast<int>(texture->GetFiltering());
			if (ImGui::ListBox(fmt::format("##filtering{}", idx).c_str(), &current, filters, IM_ARRAYSIZE(filters), 4))
			{
				for (auto texPtr : textures)
					if (texPtr != nullptr)
						texPtr->SetFiltering(static_cast<render::Texture::Filtering>(current));
				bChanged = true;
			}
		}
		ImGui::Separator();

		if (core::IsValid(texture) && lastTex != texture)
		{
			previewTex->Create(*texture);
			lastTex = texture;
		}

		const float texAspect = (float)texture->GetWidth() / (float)texture->GetHeight();
		const auto area = ImGui::GetContentRegionAvail();
		if (area.x > 1.f && texAspect > 0.01f)
		{
			if (previewTex->IsValid())
				previewTex->Draw(ImVec2{ area.x * 0.9f, (area.x * 0.9f) / texAspect });
		}
		if (bChanged)
		{
			for (auto texPtr : textures)
				if (texPtr != nullptr)
					AssetDatabase::GetInstance()->SetDirty(texPtr);
			AssetDatabase::GetInstance()->SaveAllAssets();
		}
	}
	SH_EDITOR_API void CameraInspector::RenderUI(const std::vector<core::SObject*>& objs, int idx)
	{
		game::Component* component = reinterpret_cast<game::Component*>(objs.back());
		const core::reflection::STypeInfo* currentType = &component->GetType();
		do
		{
			for (auto& prop : currentType->GetProperties())
			{
				if (prop->GetName() == core::Util::ConstexprHash("projection"))
				{
					const char* items[] = { "Perspective", "Orthographic" };
					static int current = 0;
					int* proj = prop->Get<int>(*component);
					current = *proj;
					ImGui::Text("Projection");
					if (ImGui::ListBox(fmt::format("##Projection{}", idx).c_str(), &current, items, IM_ARRAYSIZE(items), 4))
					{
						*proj = current;
						component->OnPropertyChanged(*prop);
					}
					continue;
				}
				Inspector::RenderProperty(*prop, *component, idx);
			}
			currentType = currentType->super;
		} 
		while (currentType != nullptr);
	}
	SH_EDITOR_API void TextInspector::RenderUI(const std::vector<core::SObject*>& objs, int idx)
	{
		game::TextObject* textObj = reinterpret_cast<game::TextObject*>(objs.back());
		ImGui::InputTextMultiline(fmt::format("##textBox{}", idx).c_str(), &textObj->text);
	}
}//namespace