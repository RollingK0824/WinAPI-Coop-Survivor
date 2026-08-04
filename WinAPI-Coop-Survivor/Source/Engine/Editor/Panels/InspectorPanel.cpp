#include "Engine/Core/pch.h"
#include "InspectorPanel.h"
#include "Engine/Editor/EditorSystem.h"
#include "Engine/Manager/JsonSerializer.h"
#include "Engine/Manager/ResourceManager.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Base/Component.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Render/RenderComponent.h"
void InspectorPanel::Initialize()
{
	GUISystem::GetInstance()->RegisterPanel(this);
}

void InspectorPanel::Release()
{
	GUISystem::GetInstance()->UnRegisterPanel(this);
}

void InspectorPanel::OnDrawGUI()
{
	ImGui::Begin("Inspector");
	GameObject* pSelectedObj = EditorSystem::GetInstance()->GetSelectedObject();
	if (pSelectedObj)
	{
		if (pSelectedObj->IsDead())
		{
			EditorSystem::GetInstance()->SetSelectedObject(nullptr);
			ImGui::End();
			return;
		}
		DrawHeader(pSelectedObj);
		DrawTransform(pSelectedObj);
		DrawComponents(pSelectedObj);
		DrawAddComponentButton(pSelectedObj);
	}
	ImGui::End();
}

void InspectorPanel::DrawHeader(GameObject* pObj)
{
	// Unity Style Header: [v] [GameObject Name]
	bool isActive = pObj->IsActive();
	if (ImGui::Checkbox("##IsActive", &isActive))
	{
		pObj->SetActive(isActive);
	}
	ImGui::SameLine();

	char nameBuffer[256];
	strcpy_s(nameBuffer, pObj->GetName().c_str());
	ImGui::SetNextItemWidth(-1.0f);
	if (ImGui::InputText("##Name", nameBuffer, sizeof(nameBuffer)))
	{
		pObj->SetName(nameBuffer);
	}

	ImGui::Separator();
}

// 퍼블릭 ImGui API 전용 좌측 정렬(Left-Aligned) DragFloat 위젯 (드래그 + 더블클릭 직접 입력 지원)
static bool LeftDragFloat(const char* label, float* v, float v_speed = 0.1f, const char* format = "%.3f")
{
	ImGui::PushID(label);
	ImGuiID id = ImGui::GetID("##LeftDragField");
	static ImGuiID activeEditId = 0; // 현재 키보드로 직접 입력 중인 위젯 ID

	float width = ImGui::CalcItemWidth();
	ImVec2 cursorPos = ImGui::GetCursorScreenPos();
	float height = ImGui::GetFrameHeight();
	ImVec2 size(width, height);

	bool isEditingThis = (activeEditId == id);
	bool isChanged = false;

	if (isEditingThis)
	{
		// 키보드 직접 입력 모드 (더블 클릭 시 전환)
		char buf[64];
		sprintf_s(buf, format, *v);
		ImGui::SetNextItemWidth(width);
		ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("##DirectInput", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
		{
			*v = static_cast<float>(atof(buf));
			isChanged = true;
			activeEditId = 0; // 입력 완료
		}

		if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0))
		{
			*v = static_cast<float>(atof(buf));
			activeEditId = 0; // 외부 클릭 시 종료 및 수치 반영
		}
	}
	else
	{
		// 1. 사각형 영역 버튼 이벤트 수집
		ImGui::InvisibleButton(label, size);
		bool isHovered = ImGui::IsItemHovered();
		bool isActive = ImGui::IsItemActive();

		// ★ 더블 클릭 시 키보드 직접 수치 입력 모드로 전환! ★
		if (isHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			activeEditId = id;
		}

		// 2. 마우스 드래그 조작
		if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			float delta = ImGui::GetIO().MouseDelta.x;
			if (ImGui::GetIO().KeyShift) delta *= 0.1f;
			if (ImGui::GetIO().KeyCtrl) delta *= 10.0f;
			if (delta != 0.0f)
			{
				*v += delta * v_speed;
				isChanged = true;
			}
		}

		// 3. 입력 박스 프레임 배경 그리기
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImU32 bgCol = ImGui::GetColorU32(isActive ? ImGuiCol_FrameBgActive : isHovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
		drawList->AddRectFilled(cursorPos, ImVec2(cursorPos.x + size.x, cursorPos.y + size.y), bgCol, ImGui::GetStyle().FrameRounding);

		// 4. 수치 텍스트를 박스 좌측 끝에 붙여 왼쪽 정렬로 렌더링
		char valStr[64];
		sprintf_s(valStr, format, *v);

		ImVec2 padding = ImGui::GetStyle().FramePadding;
		ImVec2 textPos(cursorPos.x + padding.x, cursorPos.y + (height - ImGui::GetTextLineHeight()) * 0.5f);
		ImU32 textCol = ImGui::GetColorU32(ImGuiCol_Text);
		drawList->AddText(textPos, textCol, valStr);
	}

	ImGui::PopID();
	return isChanged;
}

// 퍼블릭 ImGui API 전용 좌측 정렬 DragInt 위젯
static bool LeftDragInt(const char* label, int* v, float v_speed = 1.0f)
{
	float fVal = static_cast<float>(*v);
	bool changed = LeftDragFloat(label, &fVal, v_speed, "%.0f");
	if (changed)
	{
		*v = static_cast<int>(fVal);
	}
	return changed;
}

void InspectorPanel::DrawTransform(GameObject* pObj)
{
	if (ImGui::CollapsingHeader(pObj->transform.GetComponentType().data(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Columns(2, "TransformCols", false);
		float totalWidth = ImGui::GetContentRegionAvail().x;
		float col0Width = (std::max)(105.0f, totalWidth * 0.35f);
		ImGui::SetColumnWidth(0, col0Width);

		// Position (X, Y)
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Position");
		ImGui::NextColumn();

		Vector2 pos = pObj->transform.GetPosition();
		float itemW = (ImGui::GetContentRegionAvail().x - 30.0f) * 0.5f;
		if (itemW < 35.0f) itemW = 35.0f;

		ImGui::Text("X"); ImGui::SameLine();
		ImGui::SetNextItemWidth(itemW);
		if (LeftDragFloat("##PosX", &pos.x, 1.0f, "%.3f")) pObj->transform.SetPosition(pos.x, pos.y);
		ImGui::SameLine();
		ImGui::Text("Y"); ImGui::SameLine();
		ImGui::SetNextItemWidth(itemW);
		if (LeftDragFloat("##PosY", &pos.y, 1.0f, "%.3f")) pObj->transform.SetPosition(pos.x, pos.y);
		ImGui::NextColumn();

		// Rotation
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Rotation");
		ImGui::NextColumn();

		float rot = pObj->transform.GetRotation().angle;
		ImGui::SetNextItemWidth(-1.0f);
		if (LeftDragFloat("##RotAngle", &rot, 0.5f, "%.3f")) pObj->transform.SetRotation(rot);
		ImGui::NextColumn();

		// Scale (X, Y)
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Scale");
		ImGui::NextColumn();

		Vector2 scale = pObj->transform.GetScale();
		ImGui::Text("X"); ImGui::SameLine();
		ImGui::SetNextItemWidth(itemW);
		if (LeftDragFloat("##ScaleX", &scale.x, 0.05f, "%.3f")) pObj->transform.SetScale(scale.x, scale.y);
		ImGui::SameLine();
		ImGui::Text("Y"); ImGui::SameLine();
		ImGui::SetNextItemWidth(itemW);
		if (LeftDragFloat("##ScaleY", &scale.y, 0.05f, "%.3f")) pObj->transform.SetScale(scale.x, scale.y);
		ImGui::NextColumn();

		ImGui::Columns(1);
	}
}

void InspectorPanel::DrawComponents(GameObject* pObj)
{
	Component* compToRemove = nullptr;

	for (auto* comp : pObj->GetComponents())
	{
		if (!comp) continue;

		ImGui::PushID(comp);

		// Enable/Disable 체크박스
		bool isEnabled = comp->IsEnabled();
		if (ImGui::Checkbox("##IsEnabled", &isEnabled))
		{
			comp->SetEnabled(isEnabled);
		}
		ImGui::SameLine();

		// Component Header
		bool open = ImGui::CollapsingHeader(comp->GetComponentType().data(), ImGuiTreeNodeFlags_DefaultOpen);

		// 우클릭 Remove Component 팝업
		if (ImGui::BeginPopupContextItem(comp->GetComponentType().data()))
		{
			if (ImGui::MenuItem("Remove Component"))
			{
				compToRemove = comp;
			}
			ImGui::EndPopup();
		}

		if (open)
		{
			ImGui::Columns(2, "CompPropCols", false);
			float totalWidth = ImGui::GetContentRegionAvail().x;
			float col0Width = (std::max)(105.0f, totalWidth * 0.35f);
			ImGui::SetColumnWidth(0, col0Width);

			for (const auto& prop : comp->GetProperties())
			{
				ImGui::PushID(prop.name.c_str());

				ImGui::AlignTextToFramePadding();
				ImGui::Text(prop.name.c_str());
				ImGui::NextColumn();

				ImGui::SetNextItemWidth(-1.0f);

				switch (prop.type)
				{
				case PropType::Int:
					LeftDragInt(("##" + prop.name).c_str(), static_cast<int*>(prop.data), 1.0f);
					break;

				case PropType::Float:
					LeftDragFloat(("##" + prop.name).c_str(), static_cast<float*>(prop.data), 0.1f, "%.3f");
					break;

				case PropType::Bool:
					ImGui::Checkbox(("##" + prop.name).c_str(), static_cast<bool*>(prop.data));
					break;

				case PropType::String:
					{
						std::string* str = static_cast<std::string*>(prop.data);
						char buffer[256];
						strcpy_s(buffer, str->c_str());
						if (ImGui::InputText(("##" + prop.name).c_str(), buffer, sizeof(buffer)))
						{
							*str = buffer;
						}
					}
					break;

				case PropType::WString:
					{
						std::wstring* wstr = static_cast<std::wstring*>(prop.data);
						std::string str(wstr->begin(), wstr->end());
						char buffer[256];
						strcpy_s(buffer, str.c_str());
						if (ImGui::InputText(("##" + prop.name).c_str(), buffer, sizeof(buffer)))
						{
							*wstr = std::wstring(buffer, buffer + strlen(buffer));
						}
					}
					break;

				case PropType::Vector2:
					{
						Vector2* vec = static_cast<Vector2*>(prop.data);
						float itemW = (ImGui::GetContentRegionAvail().x - 30.0f) * 0.5f;
						if (itemW < 35.0f) itemW = 35.0f;

						ImGui::Text("X"); ImGui::SameLine();
						ImGui::SetNextItemWidth(itemW);
						LeftDragFloat(("##" + prop.name + "X").c_str(), &vec->x, 0.1f, "%.3f");
						ImGui::SameLine();
						ImGui::Text("Y"); ImGui::SameLine();
						ImGui::SetNextItemWidth(itemW);
						LeftDragFloat(("##" + prop.name + "Y").c_str(), &vec->y, 0.1f, "%.3f");
					}
					break;

				case PropType::Color:
					{
						D2D1_COLOR_F* color = static_cast<D2D1_COLOR_F*>(prop.data);
						float colVals[4] = { color->r, color->g, color->b, color->a };
						if (ImGui::ColorEdit4(("##" + prop.name).c_str(), colVals))
						{
							color->r = colVals[0]; color->g = colVals[1];
							color->b = colVals[2]; color->a = colVals[3];
						}
					}
					break;


				case PropType::Texture:
				{
					std::wstring* wKey = static_cast<std::wstring*>(prop.data);
					std::string keyStr(wKey->begin(), wKey->end());

					ID3D11ShaderResourceView* pSRV = ResourceManager::GetInstance()->GetTextureSRV(*wKey);
					if (pSRV) ImGui::Image((ImTextureID)pSRV, ImVec2(35.0f, 35.0f));
					else ImGui::Button("No Image", ImVec2(35.0f, 35.0f));
					ImGui::SameLine();

					std::string btnLabel = keyStr.empty() ? "Select..." : keyStr;
					if (ImGui::Button(btnLabel.c_str(), ImVec2(100.0f, 25.0f)))
					{
						ImGui::OpenPopup("TexturePickerPopup");
					}

					ImGui::SameLine();
					if (ImGui::Button("Set Native Size"))
					{
						if (RenderComponent* renderComp = dynamic_cast<RenderComponent*>(comp))
						{
							renderComp->SetNativeSize();
						}
					}
					if (ImGui::BeginPopup("TexturePickerPopup"))
					{
						auto loadedTextureKeys = ResourceManager::GetInstance()->GetLoadedTextureKeys();
						for (const auto& keyName : loadedTextureKeys)
						{
							if (ImGui::Selectable(keyName.c_str()))
							{
								*wKey = std::wstring(keyName.begin(), keyName.end());
								if (RenderComponent* renderComp = dynamic_cast<RenderComponent*>(comp))
								{
									renderComp->SetTextureKey(*wKey);
								}
							}
						}
						ImGui::EndPopup();
					}
				}
				break;
				case PropType::StringVector:
					{
						auto* vec = static_cast<std::vector<std::string>*>(prop.data);
						std::string headerText = prop.name + " (" + std::to_string(vec->size()) + ")";
						if (ImGui::TreeNode(headerText.c_str()))
						{
							int removeIdx = -1;
							for (size_t i = 0; i < vec->size(); ++i)
							{
								ImGui::PushID(static_cast<int>(i));
								char buf[256];
								strcpy_s(buf, (*vec)[i].c_str());
								ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 30.0f);
								if (ImGui::InputText(("Element " + std::to_string(i)).c_str(), buf, sizeof(buf)))
								{
									(*vec)[i] = buf;
								}
								ImGui::SameLine();
								if (ImGui::Button("-", ImVec2(20, 20))) removeIdx = static_cast<int>(i);
								ImGui::PopID();
							}
							if (removeIdx != -1) vec->erase(vec->begin() + removeIdx);
							if (ImGui::Button("+ Add Element")) vec->push_back("");
							ImGui::TreePop();
						}
					}
					break;
				}

				ImGui::NextColumn();
				ImGui::PopID();
			}

			ImGui::Columns(1);
		}

		ImGui::PopID();
	}

	if (compToRemove)
	{
		pObj->RemoveComponent(compToRemove);
	}
}

void InspectorPanel::DrawAddComponentButton(GameObject* pObj)
{
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::Button("Add Component", ImVec2(-1.0f, 30.0f)))
	{
		ImGui::OpenPopup("AddComponentPopup");
	}

	if (ImGui::BeginPopup("AddComponentPopup"))
	{
		auto compNames = JsonSerializer::GetRegisteredComponentNames();
		for (const auto& name : compNames)
		{
			if (name == EngineKey::Component::Trnasform) continue;

			if (ImGui::MenuItem(name.c_str()))
			{
				JsonSerializer::CreateComponent(name, pObj);
			}
		}
		ImGui::EndPopup();
	}
}