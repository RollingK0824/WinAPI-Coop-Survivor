#pragma once
#include "Engine/Core/Singleton.h"	
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IRenderable.h"

class GraphicManager : public Singleton<GraphicManager>, public ISystem, public IRenderable
{
	friend class Singleton<GraphicManager>;

public:
	virtual bool Initialize() override;
	virtual void Release() override;

	// IRenderable 인터페이스 (프레임 시작/마무리)
	virtual void PreRender() override;
	virtual void PostRender() override;

	// D2D 그리기 라이프사이클
	void BeginDraw();
	void EndD2DDraw();
	void Clear(const D2D1_COLOR_F& color = D2D1::ColorF(D2D1::ColorF::Black));

	// 뷰포트 오프스크린 버퍼 관리 (에디터 전용)
	void CreateViewportBuffers(uint32 width, uint32 height);
	void ResizeViewportBuffers(uint32 width, uint32 height);

	void OnResize(UINT width, UINT height);

	// 현재 활성화된 D2D RenderTarget (WITH_EDITOR 모드 시 Viewport RT 반환)
	ID2D1RenderTarget* GetRenderTarget() const
	{
#if WITH_EDITOR
		return m_pViewportRenderTarget ? m_pViewportRenderTarget : m_pRenderTarget;
#else
		return m_pRenderTarget;
#endif
	}

	// ImGui 뷰포트 패널에 제공할 SRV
	ID3D11ShaderResourceView* GetViewportSRV() const { return m_pViewportSRV; }

	ID2D1Factory* GetFactory() const { return m_pFactory; }
	IDWriteFactory* GetWriteFactory() const { return m_pWriteFactory; }

	Vector2 GetScreenSize() const
	{
		ID2D1RenderTarget* pRT = GetRenderTarget();
		return pRT ? Vector2(pRT->GetSize().width, pRT->GetSize().height) : Vector2(0, 0);
	}

	float GetScreenWidth() const {
		ID2D1RenderTarget* pRT = GetRenderTarget();
		return pRT ? pRT->GetSize().width : 0.0f;
	}

	float GetScreenHeight() const {
		ID2D1RenderTarget* pRT = GetRenderTarget();
		return pRT ? pRT->GetSize().height : 0.0f;
	}

	ID3D11Device* GetD3DDevice() const { return m_pD3DDevice; }
	ID3D11DeviceContext* GetD3DContext() const { return m_pD3DContext; }

private:
	GraphicManager() = default;
	virtual ~GraphicManager() = default;

	// D2D & DWrite 객체
	ID2D1Factory* m_pFactory = nullptr;
	IDWriteFactory* m_pWriteFactory = nullptr;
	ID2D1RenderTarget* m_pRenderTarget = nullptr; // 메인 백버퍼용 D2D Target

	// DX11 객체들
	ID3D11Device* m_pD3DDevice = nullptr;
	ID3D11DeviceContext* m_pD3DContext = nullptr;
	IDXGISwapChain* m_pSwapChain = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr; // 메인 백버퍼 RTV

	bool m_bIsD2DDrawing = false;

	// Viewport 전용 오프스크린 리소스
	ID3D11Texture2D* m_pViewportTexture = nullptr;
	ID3D11RenderTargetView* m_pViewportRTV = nullptr;
	ID3D11ShaderResourceView* m_pViewportSRV = nullptr;
	ID2D1RenderTarget* m_pViewportRenderTarget = nullptr;

	uint32 m_viewportWidth = 1920;
	uint32 m_viewportHeight = 1080;
};
