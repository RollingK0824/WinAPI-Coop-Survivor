#include "Engine/Core/pch.h"
#include "RenderSystem.h"
#include "Engine/Manager/CameraManager.h"
#include "Engine/Renderer/GraphicManager.h"
#include "Engine/Framework/Components/Core/CameraComponent.h"

bool RenderSystem::Initialize()
{
	m_commands.reserve(1000);
	return true;
}

void RenderSystem::SubmitCommand(const RenderCommand& command)
{
	m_commands.push_back(command);
}

void RenderSystem::Release()
{
	m_commands.clear();
	// 임시 vector를 만들어 swap 함수 종료 시 완전 삭제
	std::vector<RenderCommand>().swap(m_commands);
}

void RenderSystem::Render()
{
	if (m_commands.empty())return;

	// zOrder 기준으로 오름차순 정렬
	std::sort(m_commands.begin(), m_commands.end(),
		[](const RenderCommand& a, const RenderCommand& b) {
			return a.zOrder < b.zOrder;
		});

	ID2D1RenderTarget* pRT = GraphicManager::GetInstance()->GetRenderTarget();
	if (!pRT) return;

	// 기본 행렬 백업 연산 전 기본 상태 저장
	D2D1_MATRIX_3X2_F originMatrix;
	pRT->GetTransform(&originMatrix);

	for (const auto& cmd : m_commands)
	{
		if (cmd.type == RenderType::BITMAP)
		{
			DrawBitmap(pRT, cmd);
			continue;
		}

		ID2D1SolidColorBrush* pBrush = nullptr;
		pRT->CreateSolidColorBrush(cmd.color, &pBrush);
		if (!pBrush)continue;

		switch (cmd.type)
		{
		case RenderType::TEXT: DrawTextString(pRT, cmd, pBrush); break;
		case RenderType::RECT: DrawRect(pRT, cmd, pBrush); break;
		case RenderType::DEBUG_RECT: DrawDebugRect(pRT, cmd, pBrush); break;
		case RenderType::DEBUG_CIRCLE:DrawDebugCircle(pRT, cmd, pBrush); break;
		case RenderType::Debug_LINE:DrawDebugLine(pRT, cmd, pBrush); break;
		case RenderType::Debug_TEXT:DrawDebugText(pRT, cmd, pBrush); break;
		}

		pBrush->Release();
	}

	// RenderTarget 원상 복구 
	pRT->SetTransform(originMatrix);

	m_commands.clear();
}

void RenderSystem::DrawBitmap(ID2D1RenderTarget* pRT, const RenderCommand& cmd)
{
	const Sprite& sprite = cmd.bitmap.sprite;
	if (!sprite.pTexture) return;

	float srcWidth = sprite.srcRect.right - sprite.srcRect.left;
	float srcHeight = sprite.srcRect.bottom - sprite.srcRect.top;
	if (srcWidth <= 0.0f || srcHeight <= 0.0f) return;

	float renderW = (cmd.bitmap.size.x > 0.0f) ? cmd.bitmap.size.x : srcWidth;
	float renderH = (cmd.bitmap.size.y > 0.0f) ? cmd.bitmap.size.y : srcHeight;

	D2D1_POINT_2F pivot = sprite.pivot;

	float left = -renderW * pivot.x + sprite.offset.x;
	float top = -renderH * pivot.y + sprite.offset.y;

	D2D1_RECT_F destRect = D2D1::RectF(
		left,
		top,
		left + renderW,
		top + renderH
	);

	pRT->SetTransform(CalculateSRTMatrix(cmd, renderW, renderH));

	pRT->DrawBitmap(
		sprite.pTexture,
		destRect,
		cmd.bitmap.opacity,
		D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
		&sprite.srcRect
	);
}

void RenderSystem::DrawTextString(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush)
{
	if (cmd.text.pText.empty()) return;
	IDWriteFactory* pWriteFactory = GraphicManager::GetInstance()->GetWriteFactory();
	if (!pWriteFactory) return;
	IDWriteTextFormat* pTextFormat = nullptr;
	HRESULT hr = pWriteFactory->CreateTextFormat(
		L"맑은 고딕",
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		cmd.text.fontSize,
		L"ko-KR",
		&pTextFormat
	);
	if (SUCCEEDED(hr) && pTextFormat)
	{
		if (cmd.pivot.x == 0.0f)
			pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		else if (cmd.pivot.x == 1.0f)
			pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
		else
			pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

		if (cmd.pivot.y == 0.0f)
			pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		else if (cmd.pivot.y == 1.0f)
			pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
		else
			pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		float width = (cmd.srcRect.right > cmd.srcRect.left) ? (cmd.srcRect.right - cmd.srcRect.left) : 300.0f;
		float height = (cmd.srcRect.bottom > cmd.srcRect.top) ? (cmd.srcRect.bottom - cmd.srcRect.top) : 100.0f;

		pRT->SetTransform(CalculateSRTMatrix(cmd, width, height));

		float left = -width * cmd.pivot.x;
		float top = -height * cmd.pivot.y;
		D2D1_RECT_F layoutRect = D2D1::RectF(left, top, left + width, top + height);

		pRT->DrawTextW(
			cmd.text.pText.data(),
			static_cast<UINT32>(cmd.text.pText.length()),
			pTextFormat,
			layoutRect,
			pBrush
		);
		pTextFormat->Release();
	}
}

void RenderSystem::DrawRect(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush)
{
	float width = cmd.srcRect.right - cmd.srcRect.left;
	float height = cmd.srcRect.bottom - cmd.srcRect.top;
	if (width <= 0.0f) width = 100.0f;
	if (height <= 0.0f) height = 100.0f;

	pRT->SetTransform(CalculateSRTMatrix(cmd, width, height));

	float left = -width * cmd.pivot.x;
	float top = -height * cmd.pivot.y;
	D2D1_RECT_F drawRect = D2D1::RectF(left, top, left + width, top + height);

	if (cmd.shape.isFilled)
	{
		pRT->FillRectangle(drawRect, pBrush);
	}
	else
	{
		pRT->DrawRectangle(drawRect, pBrush, 1.0f);
	}
}

void RenderSystem::DrawDebugRect(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush)
{
	float width = cmd.srcRect.right - cmd.srcRect.left;
	float height = cmd.srcRect.bottom - cmd.srcRect.top;
	if (width <= 0.0f) width = 100.0f;
	if (height <= 0.0f) height = 100.0f;

	pRT->SetTransform(CalculateSRTMatrix(cmd, width, height));

	float left = -width * cmd.pivot.x;
	float top = -height * cmd.pivot.y;
	D2D1_RECT_F drawRect = D2D1::RectF(left, top, left + width, top + height);

	pRT->DrawRectangle(drawRect, pBrush, 1.0f);
}

void RenderSystem::DrawDebugCircle(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush)
{
	float radius = cmd.srcRect.left;
	float diameter = radius * 2.0f;

	pRT->SetTransform(CalculateSRTMatrix(cmd, diameter, diameter));

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(0.0f, 0.0f), radius, radius);

	if (cmd.shape.isFilled)
		pRT->FillEllipse(ellipse, pBrush);
	else
		pRT->DrawEllipse(ellipse, pBrush, 2.0f);
}

void RenderSystem::DrawDebugLine(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush)
{
	D2D1_MATRIX_3X2_F viewMatrix = D2D1::Matrix3x2F::Identity();

	if (!cmd.isUI)
	{
		viewMatrix = CameraManager::GetInstance()->GetActiveViewMatrix();
	}

	pRT->SetTransform(viewMatrix);

	D2D1_POINT_2F startPoint = D2D1::Point2F(cmd.position.x, cmd.position.y);
	D2D1_POINT_2F endPoint = D2D1::Point2F(cmd.srcRect.left, cmd.srcRect.top);

	pRT->DrawLine(startPoint, endPoint, pBrush, cmd.line.thickness);
}

void RenderSystem::DrawDebugText(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush)
{
	IDWriteFactory* pWriteFactory = GraphicManager::GetInstance()->GetWriteFactory();
	if (!pWriteFactory) return;

	IDWriteTextFormat* pTextFormat = nullptr;
	HRESULT hr = pWriteFactory->CreateTextFormat(
		L"Consolas",                // 폰트 종류
		nullptr,                    // 폰트 컬렉션 (기본값)
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		cmd.text.fontSize,               // 폰트 크기
		L"ko-KR",                   // 지역 레이아웃
		&pTextFormat
	);

	if (SUCCEEDED(hr) && pTextFormat)
	{
		D2D1_MATRIX_3X2_F transformMatrix = D2D1::Matrix3x2F::Translation(cmd.position.x, cmd.position.y);

		if (!cmd.isUI)
		{
			CameraComponent* pMainCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pMainCamera)
			{
				transformMatrix = transformMatrix * pMainCamera->GetViewMatrix();
			}
		}
		pRT->SetTransform(transformMatrix);
		D2D1_RECT_F layoutRect = D2D1::RectF(0.0f, 0.0f, 800.0f, 300.0f);
		pRT->DrawTextW(
			cmd.text.pText.data(),
			static_cast<UINT32>(cmd.text.pText.length()),
			pTextFormat,
			layoutRect,
			pBrush
		);
		pTextFormat->Release();
	}
}

D2D1_MATRIX_3X2_F RenderSystem::CalculateSRTMatrix(const RenderCommand& cmd, float width, float height)
{
	bool flipX = (cmd.type == RenderType::BITMAP) ? cmd.bitmap.flipX : false;
	bool flipY = (cmd.type == RenderType::BITMAP) ? cmd.bitmap.flipY : false;
	float scaleX = flipX ? -cmd.scaleX : cmd.scaleX;
	float scaleY = flipY ? -cmd.scaleY : cmd.scaleY;

	D2D1_MATRIX_3X2_F worldMatrix =
		D2D1::Matrix3x2F::Scale(scaleX, scaleY) *
		D2D1::Matrix3x2F::Rotation(cmd.rotation) *
		D2D1::Matrix3x2F::Translation(cmd.position.x, cmd.position.y);

	if (cmd.isUI)
	{
		return worldMatrix;
	}

	D2D1_MATRIX_3X2_F viewMatrix = CameraManager::GetInstance()->GetActiveViewMatrix();
	return worldMatrix * viewMatrix;
}