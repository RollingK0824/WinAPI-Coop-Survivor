#pragma once
#include "Engine/Manager/GUISystem.h"
#include <imgui.h>

class ViewportPanel : public IGUIPanel
{
public:
	ViewportPanel() = default;
	virtual ~ViewportPanel() override = default;

	void Initialize();
	void Release();
	virtual void OnDrawGUI() override;

	bool IsHovered() const { return m_bIsHovered; }
	bool IsFocused() const { return m_bIsFocused; }
	ImVec2 GetViewportSize() const { return m_viewportSize; }
	ImVec2 GetViewportMin() const { return m_viewportMin; }
	ImVec2 GetViewportMax() const { return m_viewportMax; }

private:
	ImVec2 m_viewportSize = { 0.0f, 0.0f };
	ImVec2 m_pendingSize = { 0.0f, 0.0f };
	ImVec2 m_viewportMin = { 0.0f, 0.0f };
	ImVec2 m_viewportMax = { 0.0f, 0.0f };

	bool m_bIsHovered = false;
	bool m_bIsFocused = false;
	bool m_bNeedResize = false;
};
