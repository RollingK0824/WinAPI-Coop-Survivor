#pragma once

float RadianToDegree(float radian);
float DegreeToRadian(float degree);

float PixelToMeter(float pixels);
float MeterToPixel(float meters);

std::wstring Utf8ToWide(const std::string& utf8Str);
std::string WideToUtf8(const std::wstring& wideStr);