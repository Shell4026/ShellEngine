#include "UI/DefaultInspector.h"
#include "UI/Inspector.h"
#include "AssetDatabase.h"

#include "imgui.h"

#include <cstdint>
#include <algorithm>
namespace sh::editor
{
	SH_EDITOR_API void GameObjectInspector::RenderUI(void* obj, int idx)
	{
		auto gameObjPtr = static_cast<game::GameObject*>(obj);
		if (!core::IsValid(gameObjPtr))
			return;

		bool bActive = gameObjPtr->activeSelf;
		if (ImGui::Checkbox("##active", &bActive))
			gameObjPtr->SetActive(bActive);
		ImGui::SameLine();
		ImGui::Text("Active");
		
		ImGui::Checkbox("##editorOnly", &gameObjPtr->bEditorOnly);
		ImGui::SameLine();
		ImGui::Text("EditorOnly");

		ImGui::Separator();

		ImGui::Text("Components");
		// 드래그 드랍으로 도중에 컴포넌트가 추가 되는 일이 발생한다.
		// 그로인해 반복자가 깨지므로 컴포넌트 배열을 복사 해둬야 한다.
		std::vector<game::Component*> components = gameObjPtr->GetComponents();

		int componentsIdx = 0;
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
				Inspector::RenderProperties(component->GetType(), *component, componentsIdx);
			++componentsIdx;
		}//for auto& component
		ImGui::Separator();

		RenderAddComponent(*gameObjPtr);
	}
	void GameObjectInspector::RenderAddComponent(game::GameObject& gameObject)
	{
		if (ImGui::Button("Add Component", { -FLT_MIN, 0.0f }))
		{
			componentItems.clear();
			auto& components = gameObject.world.componentModule.GetComponents();
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
							gameObject.AddComponent(gameObject.world.componentModule.GetComponents().at(searchName)->Create(gameObject));
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
	SH_EDITOR_API void MaterialInspector::RenderUI(void* obj, int idx)
	{
		ImGui::Separator();
		render::Material* mat = reinterpret_cast<render::Material*>(obj);
		render::Shader* shader = mat->GetShader();
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
					mat->SetShader(shader);
					AssetDatabase::GetInstance()->SetDirty(mat);
					AssetDatabase::GetInstance()->SaveAllAssets();
				}
				ImGui::EndDragDropTarget();
			}
		}
		if (core::IsValid(shader))
		{
			for (auto& [name, propInfo] : shader->GetProperties())
			{
				ImGui::Text(name.c_str());
				if (propInfo.type == core::reflection::GetType<float>())
				{
					float parameter = mat->GetProperty<float>(name).value_or(0.f);
					if (ImGui::InputFloat(("##input_" + name).c_str(), &parameter))
					{
						mat->SetProperty(name, parameter);
						AssetDatabase::GetInstance()->SetDirty(mat);
						AssetDatabase::GetInstance()->SaveAllAssets();
					}
				}
				else if (propInfo.type == core::reflection::GetType<glm::vec2>())
				{
					glm::vec2 v = mat->GetProperty<glm::vec2>(name).value_or(glm::vec2{ 0.f, 0.f });
					if (ImGui::InputFloat2(("##input_" + name).c_str(), &v[0]))
					{
						mat->SetProperty(name, v);
						AssetDatabase::GetInstance()->GetInstance()->SetDirty(mat);
						AssetDatabase::GetInstance()->SaveAllAssets();
					}
				}
				else if (propInfo.type == core::reflection::GetType<glm::vec3>())
				{
					glm::vec3 v = mat->GetProperty<glm::vec3>(name).value_or(glm::vec3{ 0.f });
					if (ImGui::InputFloat3(("##input_" + name).c_str(), &v[0]))
					{
						mat->SetProperty(name, v);
						AssetDatabase::GetInstance()->SetDirty(mat);
						AssetDatabase::GetInstance()->SaveAllAssets();
					}
				}
				else if (propInfo.type == core::reflection::GetType<glm::vec4>())
				{
					glm::vec4 v = mat->GetProperty<glm::vec4>(name).value_or(glm::vec4{ 0.f });
					if (ImGui::InputFloat4(("##input_" + name).c_str(), &v[0]))
					{
						mat->SetProperty(name, v);
						AssetDatabase::GetInstance()->SetDirty(mat);
						AssetDatabase::GetInstance()->SaveAllAssets();
					}
				}
				else if (propInfo.type == core::reflection::GetType<render::Texture>())
				{
					float iconSize = 20;
					float buttonWidth = ImGui::GetContentRegionAvail().x - iconSize;

					auto texProp = mat->GetProperty<const render::Texture*>(name);
					const render::Texture* tex = texProp.value();
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
						if (payload)
						{
							const render::Texture* texture = *reinterpret_cast<render::Texture**>(payload->Data);
							mat->SetProperty(name, texture);
							AssetDatabase::GetInstance()->SetDirty(mat);
							AssetDatabase::GetInstance()->SaveAllAssets();
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
								mat->SetConstant(name, dataValue);
								AssetDatabase::GetInstance()->SetDirty(mat);
								AssetDatabase::GetInstance()->SaveAllAssets();
							}
						}

					}
				}
				ImGui::TreePop();
			}
			
		}
		ImGui::Separator();
	}
	SH_EDITOR_API void TextureInspector::RenderUI(void* obj, int idx)
	{
		using namespace render;
		Texture* texture = reinterpret_cast<Texture*>(obj);
		Texture::TextureFormat format = texture->GetTextureFormat();

		std::string formatText = "";
		switch (format)
		{
		case Texture::TextureFormat::RGB24:
			formatText = "RGB24";
			break;
		case Texture::TextureFormat::RGBA32:
			formatText = "RGBA32";
			break;
		case Texture::TextureFormat::SRGB24:
			formatText = "SRGB24";
			break;
		case Texture::TextureFormat::SRGBA32:
			formatText = "SRGBA32";
			break;
		case Texture::TextureFormat::R8:
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
				if (format == Texture::TextureFormat::RGB24)
					texture->ChangeTextureFormat(Texture::TextureFormat::SRGB24);
				else if(format == Texture::TextureFormat::RGBA32)
					texture->ChangeTextureFormat(Texture::TextureFormat::SRGBA32);
			}
			else
			{
				if (format == Texture::TextureFormat::SRGB24)
					texture->ChangeTextureFormat(Texture::TextureFormat::RGB24);
				else if (format == Texture::TextureFormat::SRGBA32)
					texture->ChangeTextureFormat(Texture::TextureFormat::RGBA32);
			}

			AssetDatabase::GetInstance()->SetDirty(texture);
			AssetDatabase::GetInstance()->SaveAllAssets();
		}
		bool bGenerateMipmap = texture->IsGenerateMipmap();
		ImGui::Text("Generate mipmap");
		if (ImGui::Checkbox("##mipmap", &bGenerateMipmap))
		{
			texture->SetGenerateMipmap(bGenerateMipmap);
			AssetDatabase::GetInstance()->SetDirty(texture);
			AssetDatabase::GetInstance()->SaveAllAssets();
		}
		int anisoLevel = texture->GetAnisoLevel();
		ImGui::Text("Aniso");
		if (ImGui::InputInt("##Aniso", &anisoLevel))
		{
			anisoLevel = std::clamp(anisoLevel, 0, 12);
			texture->SetAnisoLevel(anisoLevel);

			AssetDatabase::GetInstance()->SetDirty(texture);
			AssetDatabase::GetInstance()->SaveAllAssets();
		}
		ImGui::Text("Filtering");
		{
			const char* filters[] = { "Linear", "Box" };
			static int current = 0;
			current = static_cast<int>(texture->GetFiltering());
			if (ImGui::ListBox(fmt::format("##filtering{}", idx).c_str(), &current, filters, IM_ARRAYSIZE(filters), 4))
			{
				texture->SetFiltering(static_cast<render::Texture::Filtering>(current));
				AssetDatabase::GetInstance()->SetDirty(texture);
				AssetDatabase::GetInstance()->SaveAllAssets();
			}
		}
	}
	SH_EDITOR_API void CameraInspector::RenderUI(void* obj, int idx)
	{
		game::Component* component = reinterpret_cast<game::Component*>(obj);
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
}//namespace