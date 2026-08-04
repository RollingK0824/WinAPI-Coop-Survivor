#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <windows.h>
#include <windowsx.h>
#include <wincodec.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <typeindex>
#include <memory>
#include <numbers>
#include <random>
#include <functional>
#include <algorithm>
#include <nlohmann/json.hpp>

#include <box2d/box2d.h>

// C++ 20 std::format
#include <format>
#include <filesystem>
namespace fs = std::filesystem;
using json = nlohmann::ordered_json;

#include "Util.h"

// transparent Blt
#pragma comment(lib, "msimg32.lib")

// 정수형을 관리하기 편한 용도
using int8 = char;		// 1byte 정수형
using int16 = short;	// 2byte 정수형
using int32 = int;		// 4byte 정수형
using int64 = long long;// 8byte 정수형

using uint8 = unsigned char;		// 1byte 정수형
using uint16 = unsigned short;		// 2byte 정수형
using uint32 = unsigned int;		// 4byte 정수형
using uint64 = unsigned long long;	// 8byte 정수형

constexpr int32 GWinSizeX = 1920;
constexpr int32 GWinSizeY = 1080;

#include <dwrite.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <d3d11.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwrite.lib")

#include <imgui.h>

#define byte win_byte
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#undef byte

#include "Types.h"

// ==========================================
// WITH_EDITOR 설정
// ==========================================
#if defined(_DEBUG)
#ifndef WITH_EDITOR
#define WITH_EDITOR 1  
#endif
#else
#undef WITH_EDITOR
#define WITH_EDITOR 0
#endif