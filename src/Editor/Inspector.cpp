﻿#include "Game/PCH.h"
#include "Inspector.h"
#include "EditorWorld.h"
#include "EditorResource.h"

#include "Core/Logger.h"
#include "Core/SObject.h"

#include "Game/GameObject.h"
#include "Game/Vector.h"

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
							std::string searchName = name;
							if(!group.empty())
								searchName = group + "/" + name;
							gameObject.AddComponent(world.componentModule.GetComponents().at(searchName)->Create(gameObject));
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

	inline void Inspector::RenderSObjectPtrProperty(const core::reflection::Property& prop, game::Component* component, const std::string& name, const std::string& typeName)
	{
		std::string typeNameSubPtr = typeName;
		typeNameSubPtr.pop_back();

		ImGui::LabelText(("##" + name).c_str(), name.c_str());
		core::SObject** parameter = prop.Get<core::SObject*>(component);

		auto icon = GetIcon(typeName);

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
		ImGui::Button(objName, ImVec2{ buttonWidth, iconSize });
		if (ImGui::BeginDragDropTarget())
		{
			// 드래그로 받는 객체의 타입 이름 == 드래그 중인 객체의 타입 이름이면 받음
			const ImGuiPayload* payload = nullptr;
			if (typeName.substr(0, 5) == "const")
			{
				std::string sub = typeName.substr(6);
				payload = ImGui::AcceptDragDropPayload(sub.c_str());
			}
			else
				payload = ImGui::AcceptDragDropPayload(typeName.c_str());
			//SH_INFO_FORMAT("payload: {}", ImGui::GetDragDropPayload()->DataType);
			if (payload)
			{
				*parameter = *reinterpret_cast<core::SObject**>(payload->Data);
				component->OnPropertyChanged(prop);
			}
			else
			{
				// 게임 오브젝트라면 컴포넌트 검사
				if (std::strcmp(ImGui::GetDragDropPayload()->DataType, "GameObject") == 0)
				{
					game::GameObject* obj = *reinterpret_cast<game::GameObject**>(ImGui::GetDragDropPayload()->Data);
					for (auto payloadComponent : obj->GetComponents())
					{
						const core::reflection::STypeInfo* componentType = &payloadComponent->GetType();
						while (componentType)
						{
							if (componentType->typeName == typeNameSubPtr)
							{
								// 요구하는 컴포넌트가 맞다면 페이로드 재설정
								ImGui::SetDragDropPayload((std::string{ componentType->typeName } + '*').c_str(), &payloadComponent, sizeof(game::Component*));
								break;
							}
							componentType = componentType->GetSuper();
						}
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
		if (icon)
		{
			ImGui::SameLine();
			ImGui::Image(*icon, ImVec2{ iconSize, iconSize });
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
				if (component->hideInspector)
					continue;
				if (ImGui::CollapsingHeader((component->GetType().name.data() + ("##" + std::to_string(idx))).data()))
				{
					auto currentType = &component->GetType();
					do
					{
						auto& props = currentType->GetProperties();
						for (auto& [name, prop] : props)
						{
							if (prop.bVisibleProperty == false)
								continue;
							std::string type{ prop.GetTypeName() };
							bool constant = prop.bConstProperty || prop.isConst;
							auto inputFlag = constant ? ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_::ImGuiInputTextFlags_None;

							// SObject 포인터 형식, 드래그 앤 드랍 기능
							if (prop.isSObjectPointer)
							{
								RenderSObjectPtrProperty(prop, component, name, type);
							}
							else
							{
								if (type == core::reflection::GetTypeName<game::Vec4>())
								{
									game::Vec4* parameter = prop.Get<game::Vec4>(component);
									float v[4] = { parameter->x, parameter->y, parameter->z };
									ImGui::LabelText(("##" + name).c_str(), name.c_str());
									if (constant)
									{
										ImGui::InputFloat4(("##" + name + std::to_string(idx)).c_str(), v, "%.3f", inputFlag);
									}
									else
									{
										if (ImGui::InputFloat4(("##" + name + std::to_string(idx)).c_str(), v))
										{
											parameter->x = v[0];
											parameter->y = v[1];
											parameter->z = v[2];
											parameter->w = v[3];
											component->OnPropertyChanged(prop);
										}
									}
								}
								else if (type == core::reflection::GetTypeName<game::Vec3>())
								{
									game::Vec3* parameter = prop.Get<game::Vec3>(component);
									float v[3] = { parameter->x, parameter->y, parameter->z };
									ImGui::LabelText(("##" + name).c_str(), name.c_str());
									if (constant)
									{
										ImGui::InputFloat3(("##" + name + std::to_string(idx)).c_str(), v, "%.3f", inputFlag);
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
								else if (type == core::reflection::GetTypeName<game::Vec2>())
								{
									game::Vec2* parameter = prop.Get<game::Vec2>(component);
									float v[2] = { parameter->x, parameter->y };
									ImGui::LabelText(("##" + name).c_str(), name.c_str());
									if (constant)
										ImGui::InputFloat2(("##" + name + std::to_string(idx)).c_str(), v, "%.3f", inputFlag);
									else
									{
										if (ImGui::InputFloat2(("##" + name + std::to_string(idx)).c_str(), v))
										{
											parameter->x = v[0];
											parameter->y = v[1];
											component->OnPropertyChanged(prop);
										}
									}
								}
								else if (type == core::reflection::GetTypeName<float>())
								{
									float* parameter = prop.Get<float>(component);
									if (constant)
										ImGui::InputFloat(("##input_" + name + std::to_string(idx)).c_str(), parameter, 0.f, 0.f, "%.3f", inputFlag);
									else
										if (ImGui::InputFloat(("##input_" + name + std::to_string(idx)).c_str(), parameter))
											component->OnPropertyChanged(prop);
								}
								else if (type == core::reflection::GetTypeName<int>())
								{
									int* parameter = prop.Get<int>(component);
									ImGui::LabelText(("##" + name).c_str(), name.c_str());
									if (constant)
										ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), parameter, 0, 0, inputFlag);
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
									if (constant)
										ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter), 0, 0, inputFlag);
									else
										if (ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter)))
											component->OnPropertyChanged(prop);
								}
								else if (type == core::reflection::GetTypeName<std::string>())
								{
									std::string* parameter = prop.Get<std::string>(component);
									ImGui::LabelText(("##" + name).c_str(), name.c_str());
									if (constant)
										ImGui::InputText(("##Input_" + name + std::to_string(idx)).c_str(), parameter, inputFlag);
									else
										if (ImGui::InputText(("##Input_" + name + std::to_string(idx)).c_str(), parameter, inputFlag))
											component->OnPropertyChanged(prop);
								}
								else if (type == core::reflection::GetTypeName<bool>())
								{
									bool* parameter = prop.Get<bool>(component);
									if (!constant)
										ImGui::Checkbox(prop.GetName().data(), parameter);
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