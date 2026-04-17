#include "UI/Inspector.h"
#include "UI/CustomInspector.h"

#include "EditorWorld.h"
#include "EditorResource.h"
#include "AssetDatabase.h"
#include "LambdaEditorCommand.h"
#include "Project.h"
#include "DragDropHelper.hpp"

#include "Core/Logger.h"
#include "Core/SObject.h"

#include "Game/GameObject.h"
#include "Game/Vector.h"

namespace sh::editor
{
	Inspector::Inspector(EditorWorld& world) :
		world(world)
	{
		customInspectorManager = CustomInspectorManager::GetInstance();
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
			bool bSameTypes = true;
			const auto& mainType = selectedObjs[0]->GetType();
			for (auto obj : selectedObjs)
			{
				if (obj->GetType() != mainType)
				{
					bSameTypes = false;
					break;
				}
			}
			if (bSameTypes)
			{
				if (auto objPtr = selectedObjs.back(); core::IsValid(objPtr))
				{
					if (!core::IsValid(objPtr))
						return;
					static std::string name;
					name = objPtr->GetName().ToString();

					ImGui::Text("%s", objPtr->GetUUID().ToString().c_str());
					ImGui::SetNextItemWidth(100);
					if (ImGui::InputText("Name", &name, ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue))
					{
						auto command = std::make_unique<LambdaEditorCommand>("Name changed", 
							[uuid = objPtr->GetUUID(), nameStr = name]()
							{
								auto sobjPtr = core::SObjectManager::GetInstance()->GetSObject(uuid);
								if (core::IsValid(sobjPtr))
									sobjPtr->SetName(nameStr);
							},
							[uuid = objPtr->GetUUID(), nameStr = objPtr->GetName().ToString()]()
							{
								auto sobjPtr = core::SObjectManager::GetInstance()->GetSObject(uuid);
								if (core::IsValid(sobjPtr))
									sobjPtr->SetName(nameStr);
							}
						);
						auto& commandHistory = world.GetProject().GetCommandHistory();
						commandHistory.Execute(std::move(command));
						AssetDatabase::GetInstance()->SetDirty(objPtr);
					}

					ImGui::Separator();

					RenderPropertiesCustomInspector(objPtr->GetType(), selectedObjs, 0);
				}
			}
			else
			{
				ImGui::Text("Selected multiple objects: %d", selectedObjs.size());
			}
		}

		ImGui::End();
	}

	SH_EDITOR_API auto Inspector::GetIcon(std::string_view typeName) -> game::GUITexture*
	{
		if (typeName == core::reflection::TypeTraits::GetTypeName<render::Mesh>())
		{
			return EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Mesh);
		}
		return nullptr;
	}
	void Inspector::RenderPropertiesCustomInspector(const core::reflection::STypeInfo& type, const std::vector<core::SObject*>& objs, int idx)
	{
		if (objs.empty())
			return;

		static CustomInspectorManager& manager = *CustomInspectorManager::GetInstance();
		auto currentType = &type;
		do
		{
			if (currentType == nullptr)
				break;

			CustomInspector* customInspector = manager.GetCustomInspector(*currentType);
			if (customInspector != nullptr)
				customInspector->RenderUI(objs, idx);
			else
			{
				auto& props = currentType->GetProperties();
				for (auto& prop : props)
				{
					if (prop->bVisibleProperty == false)
						continue;

					const std::string& propName = prop->GetName().ToString();
					// SObject 포인터 형식, 드래그 앤 드랍 기능
					if (prop->isSObjectPointer)
						RenderSObjectPtrProperty(*prop, *objs.back(), propName);
					else if (prop->isSObjectPointerContainer)
						RenderSObjPtrContainerProperty(*prop, *objs.back());
					else if (prop->isContainer && prop->type != core::reflection::GetType<std::string>()) // string도 컨테이너 취급 받아서 예외처리
						RenderContainerProperty(*prop, *objs.back(), propName);
					else
						RenderProperty(*prop, *objs.back(), idx);
				}
			}
			currentType = const_cast<core::reflection::STypeInfo*>(currentType->super);
		} while (currentType);
	}
	SH_EDITOR_API void Inspector::RenderProperties(const core::reflection::STypeInfo& type, core::SObject& obj, int idx)
	{
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
					RenderSObjectPtrProperty(*prop, obj, propName);
				else if (prop->isSObjectPointerContainer)
					RenderSObjPtrContainerProperty(*prop, obj);
				else if (prop->isContainer && prop->type != core::reflection::GetType<std::string>()) // string도 컨테이너 취급 받아서 예외처리
					RenderContainerProperty(*prop, obj, propName);
				else
					RenderProperty(*prop, obj, idx);
			}
			currentType = const_cast<core::reflection::STypeInfo*>(currentType->super);
		} while (currentType);
	}
	SH_EDITOR_API void Inspector::RenderProperty(const core::reflection::Property& prop, core::SObject& owner, int idx)
	{
		if (!prop.bVisibleProperty)
			return;
		auto& type = prop.type;
		const bool constant = prop.bConstProperty || prop.isConst;
		const std::string& name = prop.GetName().ToString();

		auto inputFlag = constant ? ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_::ImGuiInputTextFlags_None;

		bool bChanged = false;
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
					bChanged = true;
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
					bChanged = true;
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
					bChanged = true;
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
					bChanged = true;
		}
		else if (type == core::reflection::GetType<int>())
		{
			int* parameter = prop.Get<int>(owner);
			ImGui::LabelText(("##" + name).c_str(), name.c_str());
			if (constant)
				ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), parameter, 0, 0, inputFlag);
			else
				if (ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), parameter))
					bChanged = true;
		}
		else if (type == core::reflection::GetType<uint32_t>())
		{
			uint32_t* parameter = prop.Get<uint32_t>(owner);
			ImGui::LabelText(("##" + name).c_str(), name.c_str());
			if (constant)
				ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter), 0, 0, inputFlag);
			else
				if (ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter)))
					bChanged = true;
		}
		else if (type == core::reflection::GetType<std::string>())
		{
			std::string* parameter = prop.Get<std::string>(owner);
			ImGui::LabelText(("##" + name).c_str(), name.c_str());
			if (constant)
				ImGui::InputText(("##Input_" + name + std::to_string(idx)).c_str(), parameter, inputFlag);
			else
				if (ImGui::InputText(("##Input_" + name + std::to_string(idx)).c_str(), parameter, inputFlag))
					bChanged = true;
		}
		else if (type == core::reflection::GetType<bool>())
		{
			bool* parameter = prop.Get<bool>(owner);
			if (!constant)
				if (ImGui::Checkbox(name.c_str(), parameter))
					bChanged = true;
		}
		if (bChanged)
		{
			owner.OnPropertyChanged(prop);
			AssetDatabase::GetInstance()->SetDirty(&owner);
			AssetDatabase::GetInstance()->SaveAllAssets();
		}
	}
	SH_EDITOR_API void Inspector::RenderSObjectPtrProperty(const core::reflection::Property& prop, core::SObject& propertyOwner, const std::string& name,
		core::SObject** objPtr, const core::reflection::TypeInfo* type)
	{
		const core::reflection::STypeInfo& propertySTypeInfo = *core::reflection::STypeInfo::ConvertFromTypeInfo(prop.type);
		const std::string propertyTypeName{ type == nullptr ? prop.pureTypeName : type->name };
		if (type == nullptr)
			type = &prop.type;

		if (objPtr == nullptr)
			ImGui::LabelText(("##" + name).c_str(), name.c_str());
		core::SObject** parameter = objPtr;
		if (objPtr == nullptr)
			parameter = prop.Get<core::SObject*>(propertyOwner);

		auto icon = GetIcon(propertyTypeName);

		float iconSize = 20;
		float buttonWidth = ImGui::GetContentRegionAvail().x - iconSize;

		if (buttonWidth < 0)
			buttonWidth = 0;

		core::SObject* const propertyObjPtr = *parameter;

		const char* objName = "None";
		if (core::IsValid(propertyObjPtr))
		{
			if (propertyObjPtr->GetName().ToString().empty())
				objName = "Unknown";
			else
				objName = propertyObjPtr->GetName().ToString().c_str();
		}
		if (ImGui::Button(objName, ImVec2{ buttonWidth, iconSize }))
		{

		}
		if (ImGui::BeginDragDropTarget())
		{
			if (core::SObject* objPtr = dragdrop::AcceptAsset(propertySTypeInfo))
			{
				*parameter = objPtr;
				propertyOwner.OnPropertyChanged(prop);
				AssetDatabase::GetInstance()->SetDirty(&propertyOwner);
				AssetDatabase::GetInstance()->SaveAllAssets();
			}
			else
			{
				// 게임 오브젝트라면 컴포넌트 검사
				core::SObject* payloadDataPtr = *reinterpret_cast<core::SObject**>(ImGui::GetDragDropPayload()->Data);
				if (payloadDataPtr->GetType().IsChildOf(game::GameObject::GetStaticType()))
				{
					game::GameObject* gameObjPtr = static_cast<game::GameObject*>(payloadDataPtr);
					if (gameObjPtr->transform->GetType().IsChildOf(propertySTypeInfo))
					{
						ImGui::SetDragDropPayload("SObject", &gameObjPtr->transform, sizeof(core::SObject*));
						ImGui::EndDragDropTarget();
						return;
					}
					for (game::Component* const& payloadComponent : gameObjPtr->GetComponents())
					{
						if (!core::IsValid(payloadComponent))
							continue;

						const core::reflection::STypeInfo* componentType = &payloadComponent->GetType();
						if (componentType->IsChildOf(propertySTypeInfo))
						{
							ImGui::SetDragDropPayload("SObject", &payloadComponent, sizeof(core::SObject*));
							ImGui::EndDragDropTarget();
							return;
						}
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
		if (icon)
		{
			ImGui::SameLine();
			icon->Draw(ImVec2{ iconSize, iconSize });
		}
	}
	SH_EDITOR_API void Inspector::RenderSObjPtrContainerProperty(const core::reflection::Property& prop, core::SObject& propertyOwner)
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
					const core::reflection::TypeInfo& itType = it.GetType();
					const core::reflection::STypeInfo& propertySTypeInfo = *core::reflection::STypeInfo::ConvertFromTypeInfo(itType);
					const std::string propTypeName{ it.GetType().name };
					if (ImGui::Button(name, ImVec2{ buttonWidth, iconSize }))
					{
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (core::SObject* objPtr = dragdrop::AcceptAsset(propertySTypeInfo))
						{
							*obj = objPtr;
							propertyOwner.OnPropertyChanged(prop);
							AssetDatabase::GetInstance()->SetDirty(&propertyOwner);
							AssetDatabase::GetInstance()->SaveAllAssets();
						}
						else
						{
							// 게임 오브젝트라면 컴포넌트 검사
							core::SObject* payloadDataPtr = *reinterpret_cast<core::SObject**>(ImGui::GetDragDropPayload()->Data);
							if (payloadDataPtr->GetType().IsChildOf(game::GameObject::GetStaticType()))
							{
								game::GameObject* gameObjPtr = static_cast<game::GameObject*>(payloadDataPtr);
								if (gameObjPtr->transform->GetType().IsChildOf(propertySTypeInfo))
								{
									ImGui::SetDragDropPayload("SObject", &gameObjPtr->transform, sizeof(core::SObject*));
									ImGui::EndDragDropTarget();
									return;
								}
								for (game::Component* const& payloadComponent : gameObjPtr->GetComponents())
								{
									if (!core::IsValid(payloadComponent))
										continue;

									const core::reflection::STypeInfo* componentType = &payloadComponent->GetType();
									if (componentType->IsChildOf(propertySTypeInfo))
									{
										ImGui::SetDragDropPayload("SObject", &payloadComponent, sizeof(core::SObject*));
										ImGui::EndDragDropTarget();
										return;
									}
								}
							}
						}
						ImGui::EndDragDropTarget();
					}
				}
			}
			if (ImGui::Button("+"))
			{
				prop.InsertToContainer(propertyOwner, nullptr);
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
	SH_EDITOR_API void Inspector::RenderContainerProperty(const core::reflection::Property& prop, core::SObject& obj, const std::string& name)
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
				else
				{
					bool bChanged = false;
					if (it.GetType() == core::reflection::GetType<int>() || it.GetType() == core::reflection::GetType<uint32_t>())
					{
						int* parameter = it.Get<int>();
						if (ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), parameter))
							bChanged = true;
					}
					if (it.GetType() == core::reflection::GetType<int16_t>() || it.GetType() == core::reflection::GetType<uint16_t>())
					{
						int parameter = *it.Get<int16_t>();
						if (ImGui::InputInt(("##Input_" + name + std::to_string(idx)).c_str(), &parameter))
						{
							if (parameter > 0xffff)
								parameter = 0xffff;
							*it.Get<int16_t>() = static_cast<int16_t>(parameter);
							bChanged = true;
						}
					}
					else if (it.GetType() == core::reflection::GetType<float>())
					{
						float* parameter = it.Get<float>();
						if (ImGui::InputFloat(("##Input_" + name + std::to_string(idx)).c_str(), parameter))
							bChanged = true;
					}
					else if (it.GetType() == core::reflection::GetType<double>())
					{
						double* parameter = it.Get<double>();
						if (ImGui::InputFloat(("##Input_" + name + std::to_string(idx)).c_str(), reinterpret_cast<float*>(parameter)))
							bChanged = true;
					}
					else if (it.GetType() == core::reflection::GetType<bool>())
					{
						bool* parameter = it.Get<bool>();
						if (ImGui::Checkbox(("##Input_" + name + std::to_string(idx)).c_str(), parameter))
							bChanged = true;
					}
					else if (it.GetType() == core::reflection::GetType<char>())
					{
						char buffer[2] = { ' ', '\0' };
						buffer[0] = *it.Get<char>();
						if (ImGui::InputText(("##Input_" + name + std::to_string(idx)).c_str(), buffer, sizeof(buffer)))
						{
							*it.Get<char>() = buffer[0];
							bChanged = true;
						}
					}
					else if (it.GetType() == core::reflection::GetType<game::Vec2>())
					{
						game::Vec2* parameter = it.Get<game::Vec2>();
						float v[2] = { parameter->x, parameter->y };
						if (ImGui::InputFloat2(("##input_" + name + std::to_string(idx)).c_str(), v))
						{
							parameter->x = v[0];
							parameter->y = v[1];
							bChanged = true;
						}
					}
					else if (it.GetType() == core::reflection::GetType<game::Vec3>())
					{
						game::Vec3* parameter = it.Get<game::Vec3>();
						float v[3] = { parameter->x, parameter->y, parameter->z };
						if (ImGui::InputFloat3(("##input_" + name + std::to_string(idx)).c_str(), v))
						{
							parameter->x = v[0];
							parameter->y = v[1];
							parameter->z = v[2];
							bChanged = true;
						}
					}
					else if (it.GetType() == core::reflection::GetType<game::Vec4>())
					{
						game::Vec4* parameter = it.Get<game::Vec4>();
						float v[4] = { parameter->x, parameter->y, parameter->z };
						if (ImGui::InputFloat4(("##input_" + name + std::to_string(idx)).c_str(), v))
						{
							parameter->x = v[0];
							parameter->y = v[1];
							parameter->z = v[2];
							parameter->w = v[3];
							bChanged = true;
						}
					}
					else if (it.GetType() == core::reflection::GetType<std::string>())
					{
						std::string* parameter = it.Get<std::string>();
						if (ImGui::InputText(("##input_" + name + std::to_string(idx)).c_str(), parameter))
							bChanged = true;
					}
					if (bChanged)
					{
						obj.OnPropertyChanged(prop);
						AssetDatabase::GetInstance()->SetDirty(&obj);
						AssetDatabase::GetInstance()->SaveAllAssets();
					}
				}
				++idx;
			}
			if (ImGui::Button("+"))
			{
				if (*prop.containerElementType == core::reflection::GetType<int>() || *prop.containerElementType == core::reflection::GetType<uint32_t>())
					prop.InsertToContainer(obj, 0);
				else if (*prop.containerElementType == core::reflection::GetType<float>())
					prop.InsertToContainer(obj, 0.0f);
				else if (*prop.containerElementType == core::reflection::GetType<double>())
					prop.InsertToContainer(obj, 0.0);
				else if (*prop.containerElementType == core::reflection::GetType<bool>())
					prop.InsertToContainer(obj, false);
				else if (*prop.containerElementType == core::reflection::GetType<char>())
					prop.InsertToContainer(obj, '\0');
				else if (*prop.containerElementType == core::reflection::GetType<std::string>())
					prop.InsertToContainer(obj, std::string{});
				else if (*prop.containerElementType == core::reflection::GetType<game::Vec2>())
					prop.InsertToContainer(obj, game::Vec2{ 0.f, 0.f });
				else if (*prop.containerElementType == core::reflection::GetType<game::Vec3>())
					prop.InsertToContainer(obj, game::Vec3{ 0.f, 0.f, 0.f });
				else if (*prop.containerElementType == core::reflection::GetType<game::Vec4>())
					prop.InsertToContainer(obj, game::Vec4{ 0.f, 0.f, 0.f, 0.f });
				else if (*prop.containerElementType == core::reflection::GetType<int16_t>() || *prop.containerElementType == core::reflection::GetType<uint16_t>())
					prop.InsertToContainer(obj, (int16_t)0);
			}
			ImGui::SameLine();
			if (ImGui::Button("-"))
			{
				// 더 좋은 방법 나중에 고민하기
				if (prop.type.name.find("vector") != std::string_view::npos)
				{
					if (*prop.containerElementType == core::reflection::GetType<int>() || *prop.containerElementType == core::reflection::GetType<uint32_t>())
					{
						auto v = prop.Get<std::vector<int>>(obj);
						v->pop_back();
					}
					else if (*prop.containerElementType == core::reflection::GetType<float>())
					{
						auto v = prop.Get<std::vector<float>>(obj);
						v->pop_back();
					}
					else if (*prop.containerElementType == core::reflection::GetType<double>())
					{
						auto v = prop.Get<std::vector<double>>(obj);
						v->pop_back();
					}
					else if (*prop.containerElementType == core::reflection::GetType<bool>())
					{
						auto v = prop.Get<std::vector<bool>>(obj);
						v->pop_back();
					}
					else if (*prop.containerElementType == core::reflection::GetType<char>())
					{
						auto v = prop.Get<std::vector<char>>(obj);
						v->pop_back();
					}
					else if (*prop.containerElementType == core::reflection::GetType<std::string>())
					{
						auto v = prop.Get<std::vector<std::string>>(obj);
						v->pop_back();
					}
					else if (*prop.containerElementType == core::reflection::GetType<game::Vec2>())
					{
						auto v = prop.Get<std::vector<game::Vec2>>(obj);
						v->pop_back();
					}
					else if (*prop.containerElementType == core::reflection::GetType<game::Vec3>())
					{
						auto v = prop.Get<std::vector<game::Vec3>>(obj);
						v->pop_back();
					}
					else if (*prop.containerElementType == core::reflection::GetType<game::Vec4>())
					{
						auto v = prop.Get<std::vector<game::Vec4>>(obj);
						v->pop_back();
					}
					else if (*prop.containerElementType == core::reflection::GetType<int16_t>() || *prop.containerElementType == core::reflection::GetType<uint16_t>())
					{
						auto v = prop.Get<std::vector<int16_t>>(obj);
						v->pop_back();
					}
				}
			}
			ImGui::TreePop();
		}
	}
}//namespace