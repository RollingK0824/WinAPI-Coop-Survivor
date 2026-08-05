#include "Engine/Core/pch.h"
#include "ViewportPanel.h"
#include "Engine/Renderer/GraphicManager.h"
#include "Engine/Manager/CameraManager.h"

void ViewportPanel::Initialize()
{
	GUISystem::GetInstance()->RegisterPanel(this);
}

void ViewportPanel::Release()
{
	GUISystem::GetInstance()->UnRegisterPanel(this);
}

void ViewportPanel::OnDrawGUI()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImVec2 currentPanelSize = ImGui::GetContentRegionAvail();

	if ((currentPanelSize.x != m_viewportSize.x || currentPanelSize.y != m_viewportSize.y) &&
		(currentPanelSize.x > 0 && currentPanelSize.y > 0))
	{
		m_pendingSize = currentPanelSize;
		m_bNeedResize = true;
	}

	if (m_bNeedResize && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		m_viewportSize = m_pendingSize;
		GraphicManager::GetInstance()->ResizeViewportBuffers((UINT)m_viewportSize.x, (UINT)m_viewportSize.y);
		m_bNeedResize = false;
	}

	ID3D11ShaderResourceView* pSRV = GraphicManager::GetInstance()->GetViewportSRV();
	if (pSRV)
	{
		ImGui::Image((ImTextureID)pSRV, currentPanelSize);
		m_viewportMin = ImGui::GetItemRectMin();
		m_viewportMax = ImGui::GetItemRectMax();
	}

	m_bIsHovered = ImGui::IsWindowHovered();

	CameraManager* pCamMgr = CameraManager::GetInstance();
	static int s_missingCameraFrameCount = 0;
	if (!pCamMgr->IsActiveCameraValid())
	{
		s_missingCameraFrameCount++;
		if (s_missingCameraFrameCount > 3)
		{
			ImVec2 panelSize = ImGui::GetWindowSize();
			const char* warningText = "No Main Camera Rendering (Add CameraComponent to Scene)";
			ImVec2 textSize = ImGui::CalcTextSize(warningText);

			ImGui::SetCursorPos(ImVec2((panelSize.x - textSize.x) * 0.5f, (panelSize.y - textSize.y) * 0.5f));
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), warningText);
		}
	}
	else
	{
		s_missingCameraFrameCount = 0;
	}

	if (m_bIsHovered && pCamMgr->IsUsingEditorCamera())
	{
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right) || ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
		{
			ImVec2 delta = ImGui::GetIO().MouseDelta;
			pCamMgr->PanEditorCamera(Vector2(delta.x, delta.y));
		}
		float wheel = ImGui::GetIO().MouseWheel;
		if (wheel != 0.0f)
		{
			pCamMgr->ZoomEditorCamera(wheel * 0.1f);
		}
	}

	ImGui::End();
	ImGui::PopStyleVar();
}
