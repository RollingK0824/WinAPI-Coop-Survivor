#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IUpdatable.h"

class TimeManager : public Singleton<TimeManager>, public ISystem, public IUpdatable
{
	friend class Singleton<TimeManager>;

public:
	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void Update(float dt) override;

	bool AccumulateTime();
	void ConsumeFixedTick();

	float GetDeltaTime() const { return m_deltaTime; }
	float GetUnscaledDeltaTime() const { return m_unScaledDeltaTime; }
	float GetFixedDeltaTime() const { return m_fixedDeltaTime; }
	float GetGameTime() const { return m_gameTime; }
	float GetUnScaledGameTime()const { return m_unScaledGameTime; }

	float GetRealTime() const
	{
		LARGE_INTEGER currentTick;
		QueryPerformanceCounter(&currentTick);
		return static_cast<float>(currentTick.QuadPart) / static_cast<float>(m_frequency.QuadPart);
	}

	double GetRealTimeMs() const
	{
		LARGE_INTEGER currentTick;
		QueryPerformanceCounter(&currentTick);
		return (static_cast<double>(currentTick.QuadPart) * 1000.0) / static_cast<double>(m_frequency.QuadPart);
	}

	float GetTimeScale() const { return m_timeScale; }
	bool IsPaused() const { return m_bIsPaused; }
	unsigned int GetFPS() const { return m_fps; }

	void SetTimeScale(float scale) { m_timeScale = (scale < 0.0f) ? 0.0f : scale; }
	void SetPaused(bool pause) { m_bIsPaused = pause; }
	void TogglePause() { m_bIsPaused = !m_bIsPaused; }
	void ResetGameTime() { m_gameTime = 0.0f; m_unScaledGameTime = 0.0f; }

private:
	TimeManager() = default;
	virtual ~TimeManager() = default;

private:
	LARGE_INTEGER m_frequency = {};
	LARGE_INTEGER m_prevCpuTick = {};
	LARGE_INTEGER m_currentCpuTick = {};

	// 시간 계산 변수
	float m_unScaledDeltaTime = 0.0f;
	float m_deltaTime = 0.0f;
	float m_timeScale = 1.0f;
	bool m_bIsPaused = false;

	float m_gameTime = 0.0f;
	float m_unScaledGameTime = 0.0f;

	float m_fixedDeltaTime = 0.02f; // 고정 시간
	float m_accumulator = 0.0f;

	// FPS 계산용 변수
	unsigned int m_fps = 0;
	unsigned int m_frameCount = 0;
	float m_fpsFrameTime = 0.0f;

};