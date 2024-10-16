#include "Game/PCH.h"
#include "Inspector.h"
#include "EditorWorld.h"

#include "Game/GameObject.h"

namespace sh::editor
{
	SH_EDITOR_API Inspector::Inspector(game::ImGUImpl& imgui, EditorWorld& world) :
		UI(imgui),
		world(world)
	{
	}

	SH_EDITOR_API void Inspector::Update()
	{
		
	}
	SH_EDITOR_API void Inspector::Render()
	{
		ImGuiWindowFlags style =
			ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Inspector", nullptr, style);

		if (auto obj = world.GetSelectedObject(); core::IsValid(obj))
		{
			char name[30] = "";
			std::strcpy(name, obj->name.c_str());

			ImGui::SetNextItemWidth(100);
			if (ImGui::InputText("Name", name, sizeof(name)))
			{
				//std::cout << name << '\n';
			}
			ImGui::GetDrawData();

			ImGui::Separator();

			ImGui::LabelText("##ComponentsLabel", "Components");
			int idx = 0;
			for (auto& component : obj->GetComponents())
			{
				if (ImGui::CollapsingHeader((component->GetType().GetName().data() + ("##" + std::to_string(idx))).data()))
				{
					auto currentType = &component->GetType();
					do
					{
						auto& props = currentType->GetProperties();
						for (auto& prop : props)
						{
							if (prop.second.bVisible == false)
								continue;
							auto type = prop.second.GetTypeName();
							bool constant = prop.second.isConst;
							auto inputFlag = constant ? ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_::ImGuiInputTextFlags_None;
							if (type == core::reflection::GetTypeName<glm::vec3>())
							{
								glm::vec3* parameter = prop.second.Get<glm::vec3>(component);
								float v[3] = { parameter->x, parameter->y, parameter->z };
								ImGui::LabelText(("##" + prop.first).c_str(), prop.first.c_str());
								if (prop.second.isConst)
								{
									ImGui::InputFloat3(("##" + prop.first + std::to_string(idx)).c_str(), v, "%.3f", ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
								}
								else
								{
									if (ImGui::InputFloat3(("##" + prop.first + std::to_string(idx)).c_str(), v))
									{
										parameter->x = v[0];
										parameter->y = v[1];
										parameter->z = v[2];
										component->OnPropertyChanged(prop.second);
									}
								}
							}
							else if (type == core::reflection::GetTypeName<float>())
							{
								float* parameter = prop.second.Get<float>(component);
								if (ImGui::InputFloat(("##input_" + prop.first + std::to_string(idx)).c_str(), parameter))
									component->OnPropertyChanged(prop.second);
							}
							else if (type == core::reflection::GetTypeName<int>())
							{
								int* parameter = prop.second.Get<int>(component);
								ImGui::LabelText(("##" + prop.first).c_str(), prop.first.c_str());
								if (prop.second.isConst)
									ImGui::InputInt(("##Input_" + prop.first + std::to_string(idx)).c_str(), parameter, 0, 0, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
								else
									if (ImGui::InputInt(("##Input_" + prop.first + std::to_string(idx)).c_str(), parameter))
									{
										component->OnPropertyChanged(prop.second);
									}
							}
							else if (type == core::reflection::GetTypeName<uint32_t>())
							{
								uint32_t* parameter = prop.second.Get<uint32_t>(component);
								ImGui::LabelText(("##" + prop.first).c_str(), prop.first.c_str());
								if (prop.second.isConst)
									ImGui::InputInt(("##Input_" + prop.first + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter), 0, 0, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
								else
									if (ImGui::InputInt(("##Input_" + prop.first + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter)))
										component->OnPropertyChanged(prop.second);
							}
							else if (type == core::reflection::GetTypeName<std::string>())
							{
								std::string* parameter = prop.second.Get<std::string>(component);
								ImGui::LabelText(("##" + prop.first).c_str(), prop.first.c_str());
								if (ImGui::InputText(("##Input_" + prop.first + std::to_string(idx)).c_str(), parameter, inputFlag))
									component->OnPropertyChanged(prop.second);
							}
						}
						currentType = const_cast<core::reflection::STypeInfo*>(currentType->GetSuper());
					} while (currentType);
				}
				++idx;
			}//for auto& component

			ImGui::Separator();

			if (ImGui::Button("Add Component", { -FLT_MIN, 0.0f }))
				bAddComponent = !bAddComponent;
			if (bAddComponent)
			{
				static int current = -1;
				std::vector<const char*> items;
				auto& components = world.componentModule.GetComponents();
				for (auto& comp : components)
					items.push_back(comp.first.c_str());

				//ImGui::SetNextItemWidth(100);
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (ImGui::ListBox("##Components", &current, items.data(), items.size()))
				{
					std::string name = items[current];
					obj->AddComponent(components.at(name)->Create());
					bAddComponent = false;
				}

				if (ImGui::Button("Close"))
					bAddComponent = false;
			}
			//obj->SetName(name);
		}

		ImGui::End();
	}
}//namespace