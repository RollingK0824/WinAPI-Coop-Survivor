#include "Engine/Core/pch.h"
#include "GraphicManager.h"
#include "Engine/Core/GameApp.h"

bool GraphicManager::Initialize()
{
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"Direct2D Factory 생성 실패", L"Error", MB_ICONERROR);
		return false;
	}

	HWND hWnd = GameApp::GetInstance()->GetWindowHandle();
	if (!hWnd) return false;

	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT width = (rc.right - rc.left) > 0 ? (rc.right - rc.left) : 1;
	UINT height = (rc.bottom - rc.top) > 0 ? (rc.bottom - rc.top) : 1;

	// DX11 스왑체인(SwapChain) 설정
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	// DX11 디바이스 및 스왑체인 생성
	hr = D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
		nullptr, 0, D3D11_SDK_VERSION, &sd,
		&m_pSwapChain, &m_pD3DDevice, nullptr, &m_pD3DContext
	);
	if (FAILED(hr)) return false;

	ID3D11Texture2D* pBackBuffer = nullptr;

	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (FAILED(hr) || !pBackBuffer)
	{
		return false;
	}
	m_pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
	if (FAILED(hr))
	{
		pBackBuffer->Release();
		return false;
	}

	// D2D Factory 생성
	// DXGI 백버퍼를 기반으로 D2D 렌더 타겟 생성
	IDXGISurface* pDxgiSurface = nullptr;
	pBackBuffer->QueryInterface(__uuidof(IDXGISurface), (void**)&pDxgiSurface);

	if (FAILED(hr) || pDxgiSurface == nullptr)
	{
		pBackBuffer->Release();
		return false;
	}

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	hr = m_pFactory->CreateDxgiSurfaceRenderTarget(pDxgiSurface, &props, &m_pRenderTarget);

	// 사용 완료된 임시 포인터 해제
	pDxgiSurface->Release();
	pBackBuffer->Release();

	if (FAILED(hr)) return false;

	// DWrite 생성
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&m_pWriteFactory)
	);

#if WITH_EDITOR
	CreateViewportBuffers(m_viewportWidth, m_viewportHeight);
#endif

	return SUCCEEDED(hr);
}

void GraphicManager::Release()
{
	if (m_pViewportRenderTarget)	{ m_pViewportRenderTarget->Release();	m_pViewportRenderTarget = nullptr; }
	if (m_pViewportSRV)				{ m_pViewportSRV->Release();			m_pViewportSRV = nullptr; }
	if (m_pViewportRTV)				{ m_pViewportRTV->Release();			m_pViewportRTV = nullptr; }
	if (m_pViewportTexture)			{ m_pViewportTexture->Release();		m_pViewportTexture = nullptr; }

	if (m_pRenderTarget)			{ m_pRenderTarget->Release();			m_pRenderTarget = nullptr; }
	if (m_pRenderTargetView)		{ m_pRenderTargetView->Release();		m_pRenderTargetView = nullptr; }
	if (m_pSwapChain)				{ m_pSwapChain->Release();				m_pSwapChain = nullptr; }
	if (m_pD3DContext)				{ m_pD3DContext->Release();				m_pD3DContext = nullptr; }
	if (m_pD3DDevice)				{ m_pD3DDevice->Release();				m_pD3DDevice = nullptr; }

	if (m_pWriteFactory)			{ m_pWriteFactory->Release();			m_pWriteFactory = nullptr; }
	if (m_pFactory)					{ m_pFactory->Release();				m_pFactory = nullptr; }
}

void GraphicManager::PreRender()
{
	ID2D1RenderTarget* pRT = GetRenderTarget();
	if (pRT)
	{
		pRT->BeginDraw();
		pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));
		m_bIsD2DDrawing = true;
	}
}

void GraphicManager::PostRender()
{
	EndD2DDraw();

	if (m_pSwapChain)
	{
		m_pSwapChain->Present(1, 0);
		if (m_pD3DContext)
		{
			ID3D11RenderTargetView* nullRTV = nullptr;
			m_pD3DContext->OMSetRenderTargets(1, &nullRTV, nullptr);
		}
	}
}

void GraphicManager::BeginDraw()
{
	ID2D1RenderTarget* pRT = GetRenderTarget();
	if (pRT)
	{
		pRT->BeginDraw();
		m_bIsD2DDrawing = true;
	}
}

void GraphicManager::EndD2DDraw()
{
	if (!m_bIsD2DDrawing) return;
	m_bIsD2DDrawing = false;
	ID2D1RenderTarget* pRT = GetRenderTarget();
	if (pRT)
	{
		HRESULT hr = pRT->EndDraw();
		if (hr == D2DERR_RECREATE_TARGET)
		{
			Release();
			Initialize();
			return;
		}
#if WITH_EDITOR
		if (m_pD3DContext && m_pRenderTargetView)
		{
			HWND hWnd = GameApp::GetInstance()->GetWindowHandle();
			RECT rc;
			GetClientRect(hWnd, &rc);
			D3D11_VIEWPORT vp = {};
			vp.Width = static_cast<float>(rc.right - rc.left);
			vp.Height = static_cast<float>(rc.bottom - rc.top);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			m_pD3DContext->RSSetViewports(1, &vp);
			m_pD3DContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);
		}
#endif
	}
}

void GraphicManager::Clear(const D2D1_COLOR_F& color)
{
	ID2D1RenderTarget* pRT = GetRenderTarget();
	if (pRT) pRT->Clear(color);
}

void GraphicManager::CreateViewportBuffers(uint32 width, uint32 height)
{
	if (!m_pD3DDevice || width == 0 || height == 0) return;

	if (m_pViewportRenderTarget) { m_pViewportRenderTarget->Release(); m_pViewportRenderTarget = nullptr; }
	if (m_pViewportSRV) { m_pViewportSRV->Release(); m_pViewportSRV = nullptr; }
	if (m_pViewportRTV) { m_pViewportRTV->Release(); m_pViewportRTV = nullptr; }
	if (m_pViewportTexture) { m_pViewportTexture->Release(); m_pViewportTexture = nullptr; }

	m_viewportWidth = width;
	m_viewportHeight = height;

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // D2D 연동 포맷
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(m_pD3DDevice->CreateTexture2D(&texDesc, nullptr, &m_pViewportTexture))) return;
	if (FAILED(m_pD3DDevice->CreateRenderTargetView(m_pViewportTexture, nullptr, &m_pViewportRTV))) return;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	if (FAILED(m_pD3DDevice->CreateShaderResourceView(m_pViewportTexture, &srvDesc, &m_pViewportSRV))) return;

	IDXGISurface* pDxgiSurface = nullptr;
	if (SUCCEEDED(m_pViewportTexture->QueryInterface(__uuidof(IDXGISurface), (void**)&pDxgiSurface)) && pDxgiSurface)
	{
		D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
		);
		m_pFactory->CreateDxgiSurfaceRenderTarget(pDxgiSurface, &props, &m_pViewportRenderTarget);
		pDxgiSurface->Release();
	}
}

void GraphicManager::ResizeViewportBuffers(uint32 width, uint32 height)
{
	if (width == 0 || height == 0) return;
	if (m_viewportWidth == width && m_viewportHeight == height) return;
	CreateViewportBuffers(width, height);
}

void GraphicManager::OnResize(UINT width, UINT height)
{
	if (!m_pSwapChain || width == 0 || height == 0) return;

	if (m_pRenderTarget) { m_pRenderTarget->Release(); m_pRenderTarget = nullptr; }
	if (m_pRenderTargetView) { m_pRenderTargetView->Release(); m_pRenderTargetView = nullptr; }

	m_pD3DContext->OMSetRenderTargets(0, nullptr, nullptr);
	HRESULT hr = m_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr)) return;

	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (FAILED(hr) || !pBackBuffer) return;

	m_pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);

	IDXGISurface* pDxgiSurface = nullptr;
	pBackBuffer->QueryInterface(__uuidof(IDXGISurface), (void**)&pDxgiSurface);

	if (SUCCEEDED(hr) && pDxgiSurface)
	{
		D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
		);
		m_pFactory->CreateDxgiSurfaceRenderTarget(pDxgiSurface, &props, &m_pRenderTarget);
		pDxgiSurface->Release();
	}
	pBackBuffer->Release();
}

