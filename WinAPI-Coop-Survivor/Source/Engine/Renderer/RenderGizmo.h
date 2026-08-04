#pragma once
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/RenderSystem.h"

class RenderGizmo
{
public:
	// 디버그 사각형 그리기
	static void DrawRect(Vector2 pos, float width, float height, D2D1::ColorF color, bool isFilled = true, float rotation = 0.0f, int zOrder = 999)
	{
		RenderCommand cmd;
		cmd.type = RenderType::DEBUG_RECT;
		cmd.position = pos;
		cmd.srcRect = D2D1::RectF(0.0f, 0.0f, width, height); // 가로, 세로 크기를 담아 보냄
		cmd.rotation = rotation;
		cmd.color = color;
		cmd.shape.isFilled = isFilled;
		cmd.zOrder = zOrder; // 디버그 도형은 대개 최상단에 출력되도록 기본값 높게 설정

		RenderSystem::GetInstance()->SubmitCommand(cmd);
	}

	static void DrawRect(RenderCommand& cmd)
	{
		cmd.type = RenderType::DEBUG_RECT;
		RenderSystem::GetInstance()->SubmitCommand(cmd);
	}

	// 디버그 원 그리기
	static void DrawCircle(Vector2 pos, float radius, D2D1::ColorF color, bool isFilled = true, int zOrder = 999)
	{
		RenderCommand cmd;
		cmd.type = RenderType::DEBUG_CIRCLE;
		cmd.position = pos;
		cmd.srcRect.left = radius; // srcRect.left에 반지름을 보관
		cmd.color = color;
		cmd.shape.isFilled = isFilled;
		cmd.zOrder = zOrder;

		RenderSystem::GetInstance()->SubmitCommand(cmd);
	}

	// 디버그 직선 그리기
	static void DrawLine(Vector2 start, Vector2 end, D2D1::ColorF color, float thickness = 2.0f, int zOrder = 999)
	{
		RenderCommand cmd;
		cmd.type = RenderType::Debug_LINE;
		cmd.position = start;              // 시작점 (X1, Y1)
		cmd.srcRect.left = end.x;          // 끝점 X2 보관
		cmd.srcRect.top = end.y;           // 끝점 Y2 보관
		cmd.scaleX = thickness;            // 선 두께 보관
		cmd.color = color;
		cmd.zOrder = zOrder;

		RenderSystem::GetInstance()->SubmitCommand(cmd);
	}

	static void DrawDebugText(std::wstring text, Vector2 pos, float fontSize, D2D1::ColorF color = D2D1::ColorF::White, int zOrder = 999)
	{
		RenderCommand cmd;
		cmd.type = RenderType::Debug_TEXT;
		cmd.text.pText = text;
		cmd.position = pos;              // 시작점 (X1, Y1)
		cmd.scaleX = fontSize;           // 선 두께 보관
		cmd.color = color;
		cmd.zOrder = zOrder;

		RenderSystem::GetInstance()->SubmitCommand(cmd);
	}
};