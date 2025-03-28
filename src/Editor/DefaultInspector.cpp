#include "DefaultInspector.h"
#include "AssetDatabase.h"

#include "imgui.h"
namespace sh::editor
{
	void MaterialInspector::RenderUI(void* obj)
	{
		ImGui::Separator();
		render::Material* mat = reinterpret_cast<render::Material*>(obj);
		render::Shader* shader = mat->GetShader();
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
						AssetDatabase::SetDirty(mat);
						AssetDatabase::SaveAllAssets();
					}
				}
				else if (*propInfo.type == core::reflection::GetType<glm::vec2>())
				{
					glm::vec2 v = mat->GetProperty<glm::vec2>(name).value_or(glm::vec2{ 0.f, 0.f });
					if (ImGui::InputFloat2(("##input_" + name).c_str(), &v[0]))
					{
						mat->SetProperty(name, v);
						AssetDatabase::SetDirty(mat);
						AssetDatabase::SaveAllAssets();
					}
				}
				else if (*propInfo.type == core::reflection::GetType<glm::vec3>())
				{
					glm::vec3 v = mat->GetProperty<glm::vec3>(name).value_or(glm::vec3{ 0.f });
					if (ImGui::InputFloat3(("##input_" + name).c_str(), &v[0]))
					{
						mat->SetProperty(name, v);
						AssetDatabase::SetDirty(mat);
						AssetDatabase::SaveAllAssets();
					}
				}
				else if (*propInfo.type == core::reflection::GetType<glm::vec4>())
				{
					glm::vec4 v = mat->GetProperty<glm::vec4>(name).value_or(glm::vec4{ 0.f });
					if (ImGui::InputFloat4(("##input_" + name).c_str(), &v[0]))
					{
						mat->SetProperty(name, v);
						AssetDatabase::SetDirty(mat);
						AssetDatabase::SaveAllAssets();
					}
				}
				else if (*propInfo.type == core::reflection::GetType<render::Texture>())
				{
					float iconSize = 20;
					float buttonWidth = ImGui::GetContentRegionAvail().x - iconSize;

					const render::Texture* tex = mat->GetProperty<const render::Texture*>(name).value();
					std::string texName = "Empty";
					if (core::IsValid(tex))
						texName = tex->GetName().ToString();
					if (ImGui::Button(texName.c_str(), ImVec2{buttonWidth, iconSize}))
					{

					}
					if (ImGui::BeginDragDropTarget())
					{
						auto p = ImGui::GetCurrentContext()->DragDropPayload;
						auto str = std::string{ core::reflection::GetTypeName<render::Texture*>() };
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(std::string{ core::reflection::GetTypeName<render::Texture*>() }.c_str());
						if (payload)
						{
							const render::Texture* texture = *reinterpret_cast<render::Texture**>(payload->Data);
							mat->SetProperty(name, texture);
						}

						ImGui::EndDragDropTarget();
					}
				}
			}
		}
		ImGui::Separator();
	}
}//namespace