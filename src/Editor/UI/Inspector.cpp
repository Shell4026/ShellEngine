#include "UI/Inspector.h"
#include "UI/CustomInspector.h"

#include "EditorWorld.h"
#include "EditorResource.h"
#include "AssetDatabase.h"

#include "Core/Logger.h"
#include "Core/SObject.h"

#include "Game/GameObject.h"
#include "Game/Vector.h"

namespace sh::editor
{
	SH_EDITOR_API Inspector::Inspector(EditorWorld& world) :
		world(world)
	{
		customInspectorManager = CustomInspectorManager::GetInstance();
	}

	SH_EDITOR_API auto Inspector::GetIcon(std::string_view typeName) -> const game::GUITexture*
	{
		if (typeName == core::reflection::TypeTraits::GetTypeName<render::Mesh>())
		{
			return EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Mesh);
		}
		return nullptr;
	}
	SH_EDITOR_API void Inspector::RenderProperty(const core::reflection::Property& prop, core::SObject& owner, int idx)
	{
		if (!prop.bVisibleProperty)
			return;
		auto& type = prop.type;
		const bool constant = prop.bConstProperty || prop.isConst;
		const std::string& name = prop.GetName().ToString();

		auto inputFlag = constant ? ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_::ImGuiInputTextFlags_None;

		if (type == core::reflection::GetType<game::Vec4>())
		{
			game::Vec4* parameter = prop.Get<game::Vec4>(owner);
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
					owner.OnPropertyChanged(prop);
					AssetDatabase::GetInstance()->SetDirty(&owner);
					AssetDatabase::GetInstance()->SaveAllAssets();
				}
			}
		}
		else if (type == core::reflection::GetType<game::Vec3>())
		{
			game::Vec3* parameter = prop.Get<game::Vec3>(owner);
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
					owner.OnPropertyChanged(prop);
					AssetDatabase::GetInstance()->SetDirty(&owner);
					AssetDatabase::GetInstance()->SaveAllAssets();
				}
			}
		}
		else if (type == core::reflection::GetType<game::Vec2>())
		{
			game::Vec2* parameter = prop.Get<game::Vec2>(owner);
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
					owner.OnPropertyChanged(prop);
					AssetDatabase::GetInstance()->SetDirty(&owner);
					AssetDatabase::GetInstance()->SaveAllAssets();
				}
			}
		}
		else if (type == core::reflection::GetType<float>())
		{
			float* parameter = prop.Get<float>(owner);
			ImGui::Text(name.c_str());
			if (constant)
				ImGui::InputFloat(("##input_" + name + std::to_string(idx)).c_str(), parameter, 0.f, 0.f, "%.3f", inputFlag);
			else
				if (ImGui::InputFloat(("##input_" + name + std::to_string(idx)).c_str(), parameter))
				{
					owner.OnPropertyChanged(prop);
					AssetDatabase::GetInstance()->SetDirty(&owner);
					AssetDatabase::GetInstance()->SaveAllAssets();
				}
		}
		else if (type == core::reflection::GetType<int>())
		{
			int* parameter = prop.Get<int>(owner);
			ImGui::LabelText(("##" + name).c_str(), name.c_str());
			if (constant)
				ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), parameter, 0, 0, inputFlag);
			else
				if (ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), parameter))
				{
					owner.OnPropertyChanged(prop);
					AssetDatabase::GetInstance()->SetDirty(&owner);
					AssetDatabase::GetInstance()->SaveAllAssets();
				}
		}
		else if (type == core::reflection::GetType<uint32_t>())
		{
			uint32_t* parameter = prop.Get<uint32_t>(owner);
			ImGui::LabelText(("##" + name).c_str(), name.c_str());
			if (constant)
				ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter), 0, 0, inputFlag);
			else
				if (ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter)))
				{
					owner.OnPropertyChanged(prop);
					AssetDatabase::GetInstance()->SetDirty(&owner);
					AssetDatabase::GetInstance()->SaveAllAssets();
				}
		}
		else if (type == core::reflection::GetType<std::string>())
		{
			std::string* parameter = prop.Get<std::string>(owner);
			ImGui::LabelText(("##" + name).c_str(), name.c_str());
			if (constant)
				ImGui::InputText(("##Input_" + name + std::to_string(idx)).c_str(), parameter, inputFlag);
			else
				if (ImGui::InputText(("##Input_" + name + std::to_string(idx)).c_str(), parameter, inputFlag))
				{
					owner.OnPropertyChanged(prop);
					AssetDatabase::GetInstance()->SetDirty(&owner);
					AssetDatabase::GetInstance()->SaveAllAssets();
				}
		}
		else if (type == core::reflection::GetType<bool>())
		{
			bool* parameter = prop.Get<bool>(owner);
			if (!constant)
				ImGui::Checkbox(name.c_str(), parameter);
			owner.OnPropertyChanged(prop);
			AssetDatabase::GetInstance()->SetDirty(&owner);
			AssetDatabase::GetInstance()->SaveAllAssets();
		}
	}
	auto Inspector::GetComponentGroupAndName(std::string_view fullname) -> std::pair<std::string, std::string>
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
	void Inspector::RenderAddComponent(game::GameObject& gameObject)
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
	void Inspector::RenderProperties(const core::reflection::STypeInfo& type, core::SObject& obj, int idx)
	{
		ICustomInspector* customInspector = customInspectorManager->GetCustomInspector(type);
		if (customInspector)
		{
			customInspector->RenderUI(&obj, idx);
			return;
		}

		auto currentType = &type;
		do
		{
			auto& props = currentType->GetProperties();
			for (auto& prop : props)
			{
				if (prop->bVisibleProperty == false)
					continue;

				const std::string& propName = prop->GetName().ToString();
				// SObject 포인터 형식, 드래그 앤 드랍 기능
				if (prop->isSObjectPointer)
				{
					RenderSObjectPtrProperty(*prop, obj, propName);
				}
				else if (prop->isSObjectPointerContainer)
					RenderSObjPtrContainerProperty(*prop, obj);
				else if (prop->isContainer && prop->type != core::reflection::GetType<std::string>()) // string도 컨테이너 취급 받아서 예외처리
				{
					RenderContainerProperty(*prop, obj, propName);
				}
				else
				{
					RenderProperty(*prop, obj, idx);
				}
			}
			currentType = const_cast<core::reflection::STypeInfo*>(currentType->GetSuper());
		} while (currentType);
	}
	void Inspector::RenderSObjectPtrProperty(const core::reflection::Property& prop, core::SObject& propertyOwner, const std::string& name, 
		core::SObject** objPtr, const core::reflection::TypeInfo* type)
	{
		std::string typeName{ type == nullptr ? prop.pureTypeName : type->name };
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
			if (prop.type == core::reflection::GetType<const render::Mesh*>())
			{
				//world.GetEditorUI()->OpenUI<AssetExplorer>(world, imgui);
			}
		}
		if (ImGui::BeginDragDropTarget())
		{
			// 드래그로 받는 객체의 타입 이름 == 드래그 중인 객체의 타입 이름이면 받음
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(typeName.c_str());
			auto currentPayload = ImGui::GetDragDropPayload();
			if (payload)
			{
				*parameter = *reinterpret_cast<core::SObject**>(payload->Data);
				propertyOwner.OnPropertyChanged(prop);
				AssetDatabase::GetInstance()->SetDirty(&propertyOwner);
				AssetDatabase::GetInstance()->SaveAllAssets();
			}
			else
			{
				// 게임 오브젝트라면 컴포넌트 검사
				std::string gameObjTypeName{ core::reflection::GetType<game::GameObject>().name };
				if (gameObjTypeName == ImGui::GetDragDropPayload()->DataType)
				{
					game::GameObject* obj = *reinterpret_cast<game::GameObject**>(ImGui::GetDragDropPayload()->Data);

					std::vector<game::Component* const*> list{};
					for (game::Component* const& payloadComponent : obj->GetComponents())
					{
						if (!core::IsValid(payloadComponent))
							continue;

						const core::reflection::STypeInfo* componentType = &payloadComponent->GetType();
						while (componentType)
						{
							if (componentType->type.name == typeName)
							{
								// 부모가 해당 컴포넌트인 오브젝트는 여럿일 수 있으므로 후보 리스트에 넣는다.
								list.push_back(&payloadComponent);
								break;
							}
							componentType = componentType->GetSuper();
						}
					}
					bool bFind = false;
					for (game::Component* const* component : list)
					{
						const core::reflection::STypeInfo& componentType = (*component)->GetType();
						// 정확히 같은 컴포넌트라면
						if (componentType.type.name == typeName)
						{
							bFind = true;
							// 페이로드 재설정
							ImGui::SetDragDropPayload(std::string{ componentType.type.name }.c_str(), component, sizeof(game::Component*));
							break;
						}
					}
					// 정확히 같은 컴포넌트는 못 찾았으므로 제일 첫번째로 찾은 컴포넌트를 페이로드로 설정
					if (!bFind)
					{
						const core::reflection::STypeInfo& componentType = (*list[0])->GetType();
						ImGui::SetDragDropPayload(std::string{ componentType.type.name }.c_str(), list[0], sizeof(game::Component*));
					}
				}
				else
				{
					// 부모 타입하고도 일치 하는지 검사
					core::SObject** dataPtr = reinterpret_cast<core::SObject**>(currentPayload->Data);
					if (dataPtr != nullptr)
					{
						auto type = &(*dataPtr)->GetType();
						while (type != nullptr)
						{
							const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(std::string{ type->type.name }.c_str());
							if (payload != nullptr)
							{
								*parameter = *reinterpret_cast<core::SObject**>(payload->Data);
								propertyOwner.OnPropertyChanged(prop);
								AssetDatabase::GetInstance()->SetDirty(&propertyOwner);
								AssetDatabase::GetInstance()->SaveAllAssets();
							}
							type = type->GetSuper();
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
	void Inspector::RenderSObjPtrContainerProperty(const core::reflection::Property& prop, core::SObject& propertyOwner)
	{
		float iconSize = 20;
		float buttonWidth = ImGui::GetContentRegionAvail().x - iconSize;

		if (buttonWidth < 0)
			buttonWidth = 0;

		if (ImGui::TreeNodeEx(prop.GetName().ToString().c_str(), ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow))
		{
			for (auto it = prop.Begin(propertyOwner); it != prop.End(propertyOwner); ++it)
			{
				if (!it.IsPair()) // pair인 경우는 map과 unordered_map
				{
					core::SObject** obj = it.Get<core::SObject*>();
					const char* name = "None";
					if (core::IsValid(*obj))
						name = (*obj)->GetName().ToString().c_str();
					auto& itType = it.GetType();
					const std::string propTypeName{ it.GetType().name };
					if (ImGui::Button(name, ImVec2{ buttonWidth, iconSize }))
					{
					}
					if (ImGui::BeginDragDropTarget())
					{
						// 드래그로 받는 객체의 타입 == 드래그 중인 객체의 타입이면 받음
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(propTypeName.c_str());
						auto payloadt = ImGui::GetDragDropPayload();
						if (payload)
						{
							*obj = *reinterpret_cast<core::SObject**>(payload->Data);
							propertyOwner.OnPropertyChanged(prop);
							AssetDatabase::GetInstance()->SetDirty(&propertyOwner);
							AssetDatabase::GetInstance()->SaveAllAssets();
						}
						else
						{
							// 게임 오브젝트라면 컴포넌트 검사
							std::string gameObjTypeName{ core::reflection::GetType<game::GameObject>().name };
							if (gameObjTypeName == ImGui::GetDragDropPayload()->DataType)
							{
								game::GameObject* obj = *reinterpret_cast<game::GameObject**>(ImGui::GetDragDropPayload()->Data);

								std::vector<game::Component* const*> list{};
								for (game::Component* const& payloadComponent : obj->GetComponents())
								{
									if (!core::IsValid(payloadComponent))
										continue;

									const core::reflection::STypeInfo* componentType = &payloadComponent->GetType();
									while (componentType)
									{
										if (componentType->type.name == propTypeName)
										{
											// 부모가 해당 컴포넌트인 오브젝트는 여럿일 수 있으므로 후보 리스트에 넣는다.
											list.push_back(&payloadComponent);
											break;
										}
										componentType = componentType->GetSuper();
									}
								}
								bool bFind = false;
								for (game::Component*const* component : list)
								{
									const core::reflection::STypeInfo& componentType = (*component)->GetType();
									// 정확히 같은 컴포넌트라면
									if (componentType.type.name == propTypeName)
									{
										bFind = true;
										// 페이로드 재설정
										ImGui::SetDragDropPayload(std::string{ componentType.type.name }.c_str(), component, sizeof(game::Component*));
										break;
									}
								}
								// 정확히 같은 컴포넌트는 못 찾았으므로 제일 첫번째로 찾은 컴포넌트를 페이로드로 설정
								if (!bFind)
								{
									const core::reflection::STypeInfo& componentType = (*list[0])->GetType();
									ImGui::SetDragDropPayload(std::string{ componentType.type.name }.c_str(), list[0], sizeof(game::Component*));
								}
							}
						}
						ImGui::EndDragDropTarget();
					}
				}
			}
			if (ImGui::Button("+"))
			{
				// 더 좋은 방법 나중에 고민하기
				if (prop.type.name.find("vector") != std::string_view::npos)
				{
					auto v = prop.Get<std::vector<core::SObject*>>(propertyOwner);
					v->push_back(nullptr);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("-"))
			{
				// 더 좋은 방법 나중에 고민하기
				if (prop.type.name.find("vector") != std::string_view::npos)
				{
					auto v = prop.Get<std::vector<core::SObject*>>(propertyOwner);
					v->pop_back();
				}
			}

			ImGui::TreePop();
		}
	}
	void Inspector::RenderContainerProperty(const core::reflection::Property& prop, core::SObject& obj, const std::string& name)
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
							obj.OnPropertyChanged(prop);
							AssetDatabase::GetInstance()->SetDirty(&obj);
							AssetDatabase::GetInstance()->SaveAllAssets();
						}
					}
					else if (pairTypes->second == core::reflection::GetType<float>())
					{
						if (ImGui::InputFloat(("##1" + name + std::to_string(idx)).c_str(), it.GetPairSecond<float>(), 0, 0))
						{
							obj.OnPropertyChanged(prop);
							AssetDatabase::GetInstance()->SetDirty(&obj);
							AssetDatabase::GetInstance()->SaveAllAssets();
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
							obj.OnPropertyChanged(prop);
							AssetDatabase::GetInstance()->SetDirty(&obj);
							AssetDatabase::GetInstance()->SaveAllAssets();
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
							obj.OnPropertyChanged(prop);
							AssetDatabase::GetInstance()->SetDirty(&obj);
							AssetDatabase::GetInstance()->SaveAllAssets();
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
							obj.OnPropertyChanged(prop);
							AssetDatabase::GetInstance()->SetDirty(&obj);
							AssetDatabase::GetInstance()->SaveAllAssets();
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

		auto& selectedObjs = world.GetSelectedObjects();
		if (selectedObjs.size() > 0)
		{
			if (auto obj = selectedObjs[0]; core::IsValid(obj))
			{
				static std::string name;
				name = obj->GetName().ToString();

				if (obj->GetType() == game::GameObject::GetStaticType())
				{
					auto gameObj = static_cast<game::GameObject*>(obj);
					bool bActive = gameObj->activeSelf;
					if (ImGui::Checkbox("##active", &bActive))
						gameObj->SetActive(bActive);

					ImGui::SameLine();
				}
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
						std::string componentName = component->GetType().name.ToString();
						bool bOpenComponent = ImGui::CollapsingHeader((componentName.c_str() + ("##" + std::to_string(idx))).data());
						if (ImGui::BeginPopupContextItem((component->GetUUID().ToString() + "RightClickPopup").c_str()))
						{
							if (component->GetType() != game::Transform::GetStaticType())
							{
								if (ImGui::Selectable("Delete"))
								{
									component->Destroy();
								}
							}
							ImGui::EndPopup();
						}
						if (bOpenComponent && core::IsValid(component))
						{
							RenderProperties(component->GetType(), *component, idx);
						}
						++idx;
					}//for auto& component
					ImGui::Separator();

					RenderAddComponent(*static_cast<game::GameObject*>(obj));
				}
				else
					RenderProperties(obj->GetType(), *obj, 0);
			}
		}

		ImGui::End();
	}
}//namespace