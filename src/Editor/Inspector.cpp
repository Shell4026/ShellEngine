#include "Game/PCH.h"
#include "Inspector.h"
#include "EditorWorld.h"
#include "EditorResource.h"

#include "Core/Logger.h"
#include "Core/SObject.h"

#include "Game/GameObject.h"

namespace sh::editor
{
	SH_EDITOR_API Inspector::Inspector(game::ImGUImpl& imgui, EditorWorld& world) :
		UI(imgui),
		world(world)
	{
	}

	inline auto Inspector::GetIcon(std::string_view typeName) const -> const game::GUITexture*
	{
		if (typeName == core::reflection::GetTypeName<render::Mesh*>())
		{
			return EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Mesh);
		}
		return nullptr;
	}
	inline auto Inspector::GetComponentGroupAndName(std::string_view fullname) -> std::pair<std::string, std::string>
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

		return { std::move(group), std::move(name)};
	}
	inline void Inspector::RenderAddComponent(game::GameObject& gameObject)
	{
		if (ImGui::Button("Add Component", { -FLT_MIN, 0.0f }))
		{
			componentItems.clear();
			auto& components = world.componentModule.GetComponents();
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
							gameObject.AddComponent(world.componentModule.GetComponents().at(name)->Create());
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
						for (auto& [name, prop] : props)
						{
							if (prop.bVisibleProperty == false)
								continue;
							auto type = prop.GetTypeName();
							bool constant = prop.isConstProperty;
							auto inputFlag = constant ? ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_::ImGuiInputTextFlags_None;

							// SObject 포인터 형식, 드래그 앤 드랍 기능
							if (prop.isSObjectPointer)
							{
								ImGui::LabelText(("##" + name).c_str(), name.c_str());
								core::SObject** parameter = prop.Get<core::SObject*>(component);

								auto icon = GetIcon(prop.GetTypeName());

								float iconSize = 20;
								float buttonWidth = ImGui::GetContentRegionAvail().x - iconSize;

								if (buttonWidth < 0)
									buttonWidth = 0;

								const char* objName = "None";
								if (*parameter)
								{
									if ((*parameter)->editorName.empty())
										objName = "Unknown";
									else
										objName = (*parameter)->editorName.c_str();
								}
								ImGui::Button(objName, ImVec2{buttonWidth, iconSize});
								if (ImGui::BeginDragDropTarget())
								{
									const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(std::string{ prop.GetTypeName() }.c_str());
									if (payload)
									{
										*parameter = *reinterpret_cast<render::Mesh**>(payload->Data);
										component->OnPropertyChanged(prop);
									}
									ImGui::EndDragDropTarget();
								}
								if (icon)
								{
									ImGui::SameLine();
									ImGui::Image(*icon, ImVec2{ iconSize, iconSize });
								}
							}
							else
							{
								if (type == core::reflection::GetTypeName<glm::vec3>())
								{
									glm::vec3* parameter = prop.Get<glm::vec3>(component);
									float v[3] = { parameter->x, parameter->y, parameter->z };
									ImGui::LabelText(("##" + name).c_str(), name.c_str());
									if (prop.isConstProperty)
									{
										ImGui::InputFloat3(("##" + name + std::to_string(idx)).c_str(), v, "%.3f", ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
									}
									else
									{
										if (ImGui::InputFloat3(("##" + name + std::to_string(idx)).c_str(), v))
										{
											parameter->x = v[0];
											parameter->y = v[1];
											parameter->z = v[2];
											component->OnPropertyChanged(prop);
										}
									}
								}
								else if (type == core::reflection::GetTypeName<float>())
								{
									float* parameter = prop.Get<float>(component);
									if (ImGui::InputFloat(("##input_" + name + std::to_string(idx)).c_str(), parameter))
										component->OnPropertyChanged(prop);
								}
								else if (type == core::reflection::GetTypeName<int>())
								{
									int* parameter = prop.Get<int>(component);
									ImGui::LabelText(("##" + name).c_str(), name.c_str());
									if (prop.isConstProperty)
										ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), parameter, 0, 0, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
									else
										if (ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), parameter))
										{
											component->OnPropertyChanged(prop);
										}
								}
								else if (type == core::reflection::GetTypeName<uint32_t>())
								{
									uint32_t* parameter = prop.Get<uint32_t>(component);
									ImGui::LabelText(("##" + name).c_str(), name.c_str());
									if (prop.isConstProperty)
										ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter), 0, 0, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
									else
										if (ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter)))
											component->OnPropertyChanged(prop);
								}
								else if (type == core::reflection::GetTypeName<std::string>())
								{
									std::string* parameter = prop.Get<std::string>(component);
									ImGui::LabelText(("##" + name).c_str(), name.c_str());
									if (ImGui::InputText(("##Input_" + name + std::to_string(idx)).c_str(), parameter, inputFlag))
										component->OnPropertyChanged(prop);
								}
							}
						}
						currentType = const_cast<core::reflection::STypeInfo*>(currentType->GetSuper());
					} while (currentType);
				}
				++idx;
			}//for auto& component

			ImGui::Separator();

			RenderAddComponent(*obj);
		}

		ImGui::End();
	}
}//namespace