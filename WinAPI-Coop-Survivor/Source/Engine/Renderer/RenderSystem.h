#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IRenderable.h"

class RenderSystem : public Singleton<RenderSystem>, public ISystem, public IRenderable
{
	friend class Singleton<RenderSystem>;

public:
	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void Render() override;

	virtual void DrawBitmap(ID2D1RenderTarget* pRT, const RenderCommand& cmd);
	virtual void DrawDebugRect(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush);
	virtual void DrawDebugCircle(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush);
	virtual void DrawDebugLine(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush);
	virtual void DrawDebugText(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush);

	// 객체가 그리기 요청할 함수
	void SubmitCommand(const RenderCommand& command);

private:
	RenderSystem() = default;
	virtual ~RenderSystem() = default;

private:
	std::vector<RenderCommand> m_commands;

	D2D1_MATRIX_3X2_F CalculateSRTMatrix(const RenderCommand& cmd, float width, float height);
};

