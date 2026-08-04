#pragma once
#include <d2d1.h>
#include "Engine/Core/Types.h"

enum class RenderType : uint8
{
	BITMAP,
	DEBUG_RECT,
	DEBUG_CIRCLE,
	Debug_LINE,
	Debug_TEXT
};

struct BitmapParams
{
	ID2D1Bitmap* pTexture = nullptr;
	D2D1_POINT_2F offset = { 0, 0 };
	float originalWidth = 0.0f;
	float originalHeight = 0.0f;
	float opacity = 1.0f;
	bool flipX = false;
	bool flipY = false;
};

struct ShapeParams
{
	bool isFilled = true;
};

struct TextParams
{
	std::wstring_view pText;
	float fontSize = 12.0f;
};

struct LineParams
{
	D2D1_POINT_2F endPoint = { 0, 0 };
	float thickness = 1.0f;
};

struct RenderCommand
{
	// 공통 데이터
	D2D1_COLOR_F color = D2D1::ColorF(D2D1::ColorF::White);
	D2D1_RECT_F srcRect = { 0, 0, 0, 0 };
	Vector2 position = { 0.0f, 0.0f };
	D2D1_POINT_2F pivot = { 0.5f, 0.5f };
	float rotation = 0.0f;
	float scaleX = 1.0f;
	float scaleY = 1.0f;
	int16 zOrder = 0;
	RenderType type = RenderType::BITMAP;
	bool isUI = false;

	union
	{
		BitmapParams bitmap;
		ShapeParams shape;
		TextParams text;
		LineParams line;
	};

	RenderCommand()
		: color{ D2D1::ColorF(D2D1::ColorF::White) }
		, srcRect{ 0.0f, 0.0f, 0.0f, 0.0f }
		, position{ 0.0f, 0.0f }
		, pivot{ 0.5f, 0.5f }
		, rotation(0.0f)
		, scaleX(1.0f)
		, scaleY(1.0f)
		, zOrder(0)
		, type(RenderType::BITMAP)
		, isUI(false)
		, bitmap()
	{
	}
};