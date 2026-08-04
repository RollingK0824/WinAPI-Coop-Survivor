#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"

class CameraComponent;

class CameraManager : public Singleton<CameraManager>, public ISystem
{
	friend class Singleton<CameraManager>;
public:
	virtual bool Initialize() override { return true; }
	virtual void Release() override { m_pMainCamera = nullptr; }

	void SetMainCamera(CameraComponent* pCamera) { m_pMainCamera = pCamera; }
	CameraComponent* GetMainCamera() const { return m_pMainCamera; }

	D2D1_POINT_2F ScreenToWorld(D2D1_POINT_2F screenPoint)const;

	D2D1_MATRIX_3X2_F GetActiveViewMatrix() const;

	void PanEditorCamera(Vector2 delta);
	void ZoomEditorCamera(float zoomDelta);
	void ResetEditorCamera();
	Vector2 GetEditorCameraPos() const { return m_editorCamPos; }
	float GetEditorCameraZoom() const { return m_editorCamZoom; }

private:
	CameraManager() = default;
	virtual ~CameraManager() = default;

private:
	CameraComponent* m_pMainCamera = nullptr;

	Vector2 m_editorCamPos = { 0.0f, 0.0f };
	float m_editorCamZoom = 1.0f;
};