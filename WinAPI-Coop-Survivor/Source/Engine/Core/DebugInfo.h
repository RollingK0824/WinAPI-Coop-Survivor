#pragma once
#include "Engine/Core/pch.h"

struct DebugInfo
{
	uint32 fps = 0;
	float gameTime = 0.0f;
	std::string roleStr = "OFFLINE";
	uint32 netID = 0;
	float ping = 0.0f;
	bool bIsConnected = false;
};