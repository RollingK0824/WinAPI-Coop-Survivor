#include "Engine/Core/pch.h"
#include "CameraManager.h"
#include "Engine/Core/EngineKernel.h"
#include "Engine/Framework/Components/Core/CameraComponent.h"
#include "Engine/Renderer/GraphicManager.h"

D2D1_MATRIX_3X2_F CameraManager::GetActiveViewMatrix() const
{
    if (m_pMainCamera)
    {
        return m_pMainCamera->GetViewMatrix();
    }
    // 2. 메인 카메라이 없을 경우 에디터 전용 카메라 View Matrix 계산
    float screenWidth = GraphicManager::GetInstance()->GetScreenWidth();
    float screenHeight = GraphicManager::GetInstance()->GetScreenHeight();
    D2D1_POINT_2F screenCenter = D2D1::Point2F(screenWidth * 0.5f, screenHeight * 0.5f);
    D2D1_MATRIX_3X2_F matTrans = D2D1::Matrix3x2F::Translation(-m_editorCamPos.x, -m_editorCamPos.y);
    D2D1_MATRIX_3X2_F matScale = D2D1::Matrix3x2F::Scale(m_editorCamZoom, m_editorCamZoom, D2D1::Point2F(0.0f, 0.0f));
    D2D1_MATRIX_3X2_F matCenter = D2D1::Matrix3x2F::Translation(screenCenter.x, screenCenter.y);
    return matTrans * matScale * matCenter;
}

D2D1_POINT_2F CameraManager::ScreenToWorld(D2D1_POINT_2F screenPoint) const
{
    if (m_pMainCamera)
    {
        return m_pMainCamera->ScreenToWorldPoint(screenPoint);
    }
    D2D1_MATRIX_3X2_F viewMat = GetActiveViewMatrix();
    D2D1_MATRIX_3X2_F invViewMat = viewMat;
    if (D2D1InvertMatrix(&invViewMat))
    {
        return D2D1::Matrix3x2F::ReinterpretBaseType(&invViewMat)->TransformPoint(screenPoint);
    }
    return screenPoint;
}

void CameraManager::PanEditorCamera(Vector2 delta)
{
    // 줌 배율을 고려한 이동 델타 처리
    m_editorCamPos.x -= (delta.x / m_editorCamZoom);
    m_editorCamPos.y -= (delta.y / m_editorCamZoom);
}

void CameraManager::ZoomEditorCamera(float zoomDelta)
{
    m_editorCamZoom += zoomDelta;
    if (m_editorCamZoom < 0.1f) m_editorCamZoom = 0.1f;
    if (m_editorCamZoom > 5.0f) m_editorCamZoom = 5.0f;
}

void CameraManager::ResetEditorCamera()
{
    m_editorCamPos = { 0.0f, 0.0f };
    m_editorCamZoom = 1.0f;
}