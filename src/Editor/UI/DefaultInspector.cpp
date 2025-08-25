#include "UI/DefaultInspector.h"
#include "UI/Inspector.h"
#include "AssetDatabase.h"

#include "imgui.h"

#include <cstdint>
#include <algorithm>
namespace sh::editor
{
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
				if (*propInfo.type == core::reflection::GetType<float>())
				{
					float parameter = mat->GetProperty<float>(name).value_or(0.f);
					if (ImGui::InputFloat(("##input_" + name).c_str(), &parameter))
					{
						mat->SetProperty(name, parameter);
						AssetDatabase::GetInstance()->SetDirty(mat);
						AssetDatabase::GetInstance()->SaveAllAssets();
					}
				}
				else if (*propInfo.type == core::reflection::GetType<glm::vec2>())
				{
					glm::vec2 v = mat->GetProperty<glm::vec2>(name).value_or(glm::vec2{ 0.f, 0.f });
					if (ImGui::InputFloat2(("##input_" + name).c_str(), &v[0]))
					{
						mat->SetProperty(name, v);
						AssetDatabase::GetInstance()->GetInstance()->SetDirty(mat);
						AssetDatabase::GetInstance()->SaveAllAssets();
					}
				}
				else if (*propInfo.type == core::reflection::GetType<glm::vec3>())
				{
					glm::vec3 v = mat->GetProperty<glm::vec3>(name).value_or(glm::vec3{ 0.f });
					if (ImGui::InputFloat3(("##input_" + name).c_str(), &v[0]))
					{
						mat->SetProperty(name, v);
						AssetDatabase::GetInstance()->SetDirty(mat);
						AssetDatabase::GetInstance()->SaveAllAssets();
					}
				}
				else if (*propInfo.type == core::reflection::GetType<glm::vec4>())
				{
					glm::vec4 v = mat->GetProperty<glm::vec4>(name).value_or(glm::vec4{ 0.f });
					if (ImGui::InputFloat4(("##input_" + name).c_str(), &v[0]))
					{
						mat->SetProperty(name, v);
						AssetDatabase::GetInstance()->SetDirty(mat);
						AssetDatabase::GetInstance()->SaveAllAssets();
					}
				}
				else if (*propInfo.type == core::reflection::GetType<render::Texture>())
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
		}
		ImGui::Separator();
	}
	SH_EDITOR_API void TextureInspector::RenderUI(void* obj, int idx)
	{
		using namespace render;
		Texture* texture = reinterpret_cast<Texture*>(obj);
		ImGui::Text("SRGB");
		bool bSRGB = texture->IsSRGB();
		if (ImGui::Checkbox("##SRGB", &bSRGB))
		{
			Texture::TextureFormat format = texture->GetTextureFormat();
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
		int anisoLevel = texture->GetAnisoLevel();
		ImGui::Text("Aniso");
		if (ImGui::InputInt("##Aniso", &anisoLevel))
		{
			anisoLevel = std::clamp(anisoLevel, 0, 12);
			texture->SetAnisoLevel(anisoLevel);

			AssetDatabase::GetInstance()->SetDirty(texture);
			AssetDatabase::GetInstance()->SaveAllAssets();
		}
	}
	SH_EDITOR_API void CameraInspector::RenderUI(void* obj, int idx)
	{
		game::Component* component = reinterpret_cast<game::Component*>(obj);
		auto currentType = &component->GetType();
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
			currentType = currentType->GetSuper();
		} 
		while (currentType != nullptr);
	}
}//namespace