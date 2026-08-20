#include "Engine/Core/pch.h"
#include "Util.h"

float RadianToDegree(float radian)
{
	float degree = radian * (180.0f / std::numbers::pi_v<float>);

	degree = fmodf(degree, 360.0f);
	if (degree < 0.0f)
		degree += 360.0f;

	return degree;
}

float DegreeToRadian(float degree)
{
	float radian = degree * (std::numbers::pi_v<float> / 180.0f);

	radian = fmodf(radian, std::numbers::pi_v<float> *2);
	if (radian < 0.0f)
		radian += std::numbers::pi_v<float> *2;

	return radian;
}

float PixelToMeter(float pixels) 
{ 
	return pixels / PTM_RATIO; 
}

float MeterToPixel(float meters) 
{
	return meters * PTM_RATIO; 
}

std::wstring Utf8ToWide(const std::string& utf8Str)
{
	if (utf8Str.empty()) return L"";
	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), static_cast<int>(utf8Str.size()), nullptr, 0);
	if (sizeNeeded <= 0) return L"";
	std::wstring wstr(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), static_cast<int>(utf8Str.size()), &wstr[0], sizeNeeded);
	return wstr;
}

std::string WideToUtf8(const std::wstring& wideStr)
{
	if (wideStr.empty()) return "";
	int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wideStr.data(), static_cast<int>(wideStr.size()), nullptr, 0, nullptr, nullptr);
	if (sizeNeeded <= 0) return "";
	std::string str(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, wideStr.data(), static_cast<int>(wideStr.size()), &str[0], sizeNeeded, nullptr, nullptr);
	return str;
}
