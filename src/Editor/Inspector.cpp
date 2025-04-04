#include "Game/PCH.h"
#include "Inspector.h"
#include "EditorWorld.h"
#include "EditorResource.h"
#include "AssetDatabase.h"
#include "CustomInspector.h"

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
		customInspectorManager = CustomInspectorManager::GetInstance();
	}

	SH_EDITOR_API auto Inspector::GetIcon(std::string_view typeName) -> const game::GUITexture*
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
	inline void Inspector::RenderProperties(const core::reflection::STypeInfo* type, core::SObject* obj, int idx)
	{
		if (type == nullptr)
			return;

		ICustomInspector* customInspector = customInspectorManager->GetCustomInspector(type);
		if (customInspector)
		{
			customInspector->RenderUI(obj);
			return;
		}

		auto currentType = type;
		do
		{
			auto& props = currentType->GetProperties();
			for (auto& prop : props)
			{
				if (prop->bVisibleProperty == false)
					continue;
				const core::reflection::TypeInfo& type = prop->type;
				bool constant = prop->bConstProperty || prop->isConst;
				std::string name = prop->GetName().ToString();

				auto inputFlag = constant ? ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_::ImGuiInputTextFlags_None;

				// SObject 포인터 형식, 드래그 앤 드랍 기능
				if (prop->isSObjectPointer)
				{
					RenderSObjectPtrProperty(*prop, obj, name);
				}
				if (prop->isContainer)
				{
					RenderContainerProperty(*prop, obj, name);
				}
				else
				{
					if (type == core::reflection::GetType<game::Vec4>())
					{
						game::Vec4* parameter = prop->Get<game::Vec4>(obj);
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
								obj->OnPropertyChanged(*prop);
								AssetDatabase::SetDirty(obj);
								AssetDatabase::SaveAllAssets();
							}
						}
					}
					else if (type == core::reflection::GetType<game::Vec3>())
					{
						game::Vec3* parameter = prop->Get<game::Vec3>(obj);
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
								obj->OnPropertyChanged(*prop);
								AssetDatabase::SetDirty(obj);
								AssetDatabase::SaveAllAssets();
							}
						}
					}
					else if (type == core::reflection::GetType<game::Vec2>())
					{
						game::Vec2* parameter = prop->Get<game::Vec2>(obj);
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
								obj->OnPropertyChanged(*prop);
								AssetDatabase::SetDirty(obj);
								AssetDatabase::SaveAllAssets();
							}
						}
					}
					else if (type == core::reflection::GetType<float>())
					{
						float* parameter = prop->Get<float>(obj);
						if (constant)
							ImGui::InputFloat(("##input_" + name + std::to_string(idx)).c_str(), parameter, 0.f, 0.f, "%.3f", inputFlag);
						else
							if (ImGui::InputFloat(("##input_" + name + std::to_string(idx)).c_str(), parameter))
							{
								obj->OnPropertyChanged(*prop);
								AssetDatabase::SetDirty(obj);
								AssetDatabase::SaveAllAssets();
							}
					}
					else if (type == core::reflection::GetType<int>())
					{
						int* parameter = prop->Get<int>(obj);
						ImGui::LabelText(("##" + name).c_str(), name.c_str());
						if (constant)
							ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), parameter, 0, 0, inputFlag);
						else
							if (ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), parameter))
							{
								obj->OnPropertyChanged(*prop);
								AssetDatabase::SetDirty(obj);
								AssetDatabase::SaveAllAssets();
							}
					}
					else if (type == core::reflection::GetType<uint32_t>())
					{
						uint32_t* parameter = prop->Get<uint32_t>(obj);
						ImGui::LabelText(("##" + name).c_str(), name.c_str());
						if (constant)
							ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter), 0, 0, inputFlag);
						else
							if (ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter)))
							{
								obj->OnPropertyChanged(*prop);
								AssetDatabase::SetDirty(obj);
								AssetDatabase::SaveAllAssets();
							}
					}
					else if (type == core::reflection::GetType<std::string>())
					{
						std::string* parameter = prop->Get<std::string>(obj);
						ImGui::LabelText(("##" + name).c_str(), name.c_str());
						if (constant)
							ImGui::InputText(("##Input_" + name + std::to_string(idx)).c_str(), parameter, inputFlag);
						else
							if (ImGui::InputText(("##Input_" + name + std::to_string(idx)).c_str(), parameter, inputFlag))
							{
								obj->OnPropertyChanged(*prop);
								AssetDatabase::SetDirty(obj);
								AssetDatabase::SaveAllAssets();
							}
					}
					else if (type == core::reflection::GetType<bool>())
					{
						bool* parameter = prop->Get<bool>(obj);
						if (!constant)
							ImGui::Checkbox(name.c_str(), parameter);
						obj->OnPropertyChanged(*prop);
						AssetDatabase::SetDirty(obj);
						AssetDatabase::SaveAllAssets();
					}
				}
			}
			currentType = const_cast<core::reflection::STypeInfo*>(currentType->GetSuper());
		} while (currentType);
	}
	inline void Inspector::RenderSObjectPtrProperty(const core::reflection::Property& prop, core::SObject* propertyOwner, const std::string& name, 
		core::SObject** objPtr, const core::reflection::TypeInfo* type)
	{
		std::string typeName{ type == nullptr ? prop.type.name : type->name };
		if (type == nullptr)
			type = &prop.type;

		if (objPtr == nullptr)
			ImGui::LabelText(("##" + name).c_str(), name.c_str());
		core::SObject** parameter = objPtr;
		if (objPtr == nullptr)
			parameter = prop.Get<core::SObject*>(propertyOwner);

		auto icon = GetIcon(typeName);

		float iconSize = 20;
		float buttonWidth = ImGui::GetContentRegionAvail().x - iconSize;

		if (buttonWidth < 0)
			buttonWidth = 0;

		const char* objName = "None";
		if (*parameter)
		{
			if ((*parameter)->GetName().ToString().empty())
				objName = "Unknown";
			else
				objName = (*parameter)->GetName().ToString().c_str();
		}
		if (ImGui::Button(objName, ImVec2{ buttonWidth, iconSize }))
		{
			
		}
		if (ImGui::BeginDragDropTarget())
		{
			// 드래그로 받는 객체의 타입 이름 == 드래그 중인 객체의 타입 이름이면 받음
			const ImGuiPayload* payload = nullptr;
			if (typeName.substr(0, 5) == "const")
			{
				std::string sub{ typeName.substr(6) };
				payload = ImGui::AcceptDragDropPayload(sub.c_str());
			}
			else
				payload = ImGui::AcceptDragDropPayload(typeName.c_str());
			//SH_INFO_FORMAT("payload: {}", ImGui::GetDragDropPayload()->DataType);
			if (payload)
			{
				*parameter = *reinterpret_cast<core::SObject**>(payload->Data);
				propertyOwner->OnPropertyChanged(prop);
				AssetDatabase::SetDirty(propertyOwner);
				AssetDatabase::SaveAllAssets();
			}
			else
			{
				// 게임 오브젝트라면 컴포넌트 검사
				if (std::strcmp(ImGui::GetDragDropPayload()->DataType, "GameObject") == 0)
				{
					game::GameObject* obj = *reinterpret_cast<game::GameObject**>(ImGui::GetDragDropPayload()->Data);
					for (auto payloadComponent : obj->GetComponents())
					{
						if (!core::IsValid(payloadComponent))
							continue;

						const core::reflection::STypeInfo* componentType = &payloadComponent->GetType();
						while (componentType)
						{
							if (componentType->type == *type)
							{
								// 요구하는 컴포넌트가 맞다면 페이로드 재설정
								ImGui::SetDragDropPayload((std::string{ componentType->name } + '*').c_str(), &payloadComponent, sizeof(game::Component*));
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
	inline void Inspector::RenderContainerProperty(const core::reflection::Property& prop, core::SObject* obj, const std::string& name)
	{
		float itemWidth = ImGui::GetContentRegionAvail().x / 2.f - 1;
		if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow))
		{
			int idx = 0;
			for (auto it = prop.Begin(obj); it != prop.End(obj); ++it)
			{
				// map, hashmap
				if (it.IsPair())
				{
					auto pairTypes = it.GetPairType();
					ImGui::SetNextItemWidth(itemWidth);
					if (pairTypes->first == core::reflection::GetType<std::string>())
						ImGui::LabelText(("##0" + name + std::to_string(idx)).c_str(), it.GetPairFirst<std::string>()->c_str());

					ImGui::SameLine();

					ImGui::SetNextItemWidth(itemWidth);
					if (pairTypes->second.isSObjectPointer)
					{
						RenderSObjectPtrProperty(prop, obj, name, it.GetPairSecond<core::SObject*>(), &pairTypes->second);
					}
					else if (pairTypes->second == core::reflection::GetType<int>())
					{
						if (ImGui::InputInt(("##1" + name + std::to_string(idx)).c_str(), it.GetPairSecond<int>(), 0, 0))
						{
							obj->OnPropertyChanged(prop);
							AssetDatabase::SetDirty(obj);
							AssetDatabase::SaveAllAssets();
						}
					}
					else if (pairTypes->second == core::reflection::GetType<float>())
					{
						if (ImGui::InputFloat(("##1" + name + std::to_string(idx)).c_str(), it.GetPairSecond<float>(), 0, 0))
						{
							obj->OnPropertyChanged(prop);
							AssetDatabase::SetDirty(obj);
							AssetDatabase::SaveAllAssets();
						}
					}
					else if (pairTypes->second == core::reflection::GetType<glm::vec2>())
					{
						glm::vec2* v = it.GetPairSecond<glm::vec2>();
						float input[2] = { v->x, v->y };
						if (ImGui::InputFloat2(("##1" + name + std::to_string(idx)).c_str(), input, 0, 0))
						{
							v->x = input[0];
							v->y = input[1];
							obj->OnPropertyChanged(prop);
							AssetDatabase::SetDirty(obj);
							AssetDatabase::SaveAllAssets();
						}
					}
					else if (pairTypes->second == core::reflection::GetType<glm::vec3>())
					{
						glm::vec3* v = it.GetPairSecond<glm::vec3>();
						float input[3] = { v->x, v->y, v->z };
						if (ImGui::InputFloat2(("##1" + name + std::to_string(idx)).c_str(), input, 0, 0))
						{
							v->x = input[0];
							v->y = input[1];
							v->z = input[2];
							obj->OnPropertyChanged(prop);
							AssetDatabase::SetDirty(obj);
							AssetDatabase::SaveAllAssets();
						}
					}
					else if (pairTypes->second == core::reflection::GetType<glm::vec4>())
					{
						glm::vec4* v = it.GetPairSecond<glm::vec4>();
						float input[4] = { v->x, v->y, v->z, v->w };
						if (ImGui::InputFloat2(("##1" + name + std::to_string(idx)).c_str(), input, 0, 0))
						{
							v->x = input[0];
							v->y = input[1];
							v->z = input[2];
							v->w = input[3];
							obj->OnPropertyChanged(prop);
							AssetDatabase::SetDirty(obj);
							AssetDatabase::SaveAllAssets();
						}
					}
				}
				++idx;
			}
			ImGui::TreePop();
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
			static std::string name;
			name = obj->GetName().ToString();

			ImGui::SetNextItemWidth(100);
			if (ImGui::InputText("Name", &name))
				obj->SetName(name);

			ImGui::Separator();

			if (obj->GetType() == game::GameObject::GetStaticType())
			{
				ImGui::LabelText("##ComponentsLabel", "Components");
				int idx = 0;
				// 드래그 드랍으로 도중에 컴포넌트가 추가 되는 일이 발생한다.
				// 그로인해 반복자가 깨지므로 컴포넌트 배열을 복사 해둬야 한다.
				std::vector<game::Component*> components = static_cast<game::GameObject*>(obj)->GetComponents();
				for (auto component : components)
				{
					if (!core::IsValid(component))
						continue;
					if (component->hideInspector)
						continue;
					if (ImGui::CollapsingHeader((component->GetType().name.ToString().c_str() + ("##" + std::to_string(idx))).data()))
					{
						RenderProperties(&component->GetType(), component, idx);
					}
					++idx;
				}//for auto& component
				ImGui::Separator();

				RenderAddComponent(*static_cast<game::GameObject*>(obj));
			}
			else
				RenderProperties(&obj->GetType(), obj, 0);
		}

		ImGui::End();
	}
}//namespace