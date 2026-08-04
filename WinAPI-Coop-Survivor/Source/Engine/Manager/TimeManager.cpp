#include "Engine/Core/pch.h"
#include "TimeManager.h"

bool TimeManager::Initialize()
{
    if (!QueryPerformanceFrequency(&m_frequency))
    {
        return false;
    }
   
    QueryPerformanceCounter(&m_prevCpuTick);

    m_unScaledDeltaTime = 0.0f;
    m_deltaTime = 0.0f;
    m_timeScale = 1.0f;
    m_bIsPaused = false;
    m_gameTime = 0.0f;
    m_unScaledGameTime = 0.0f;

    return true;
}

void TimeManager::Update(float dt)
{
    // 현재 프레임의 틱 카운트 가져오기
    QueryPerformanceCounter(&m_currentCpuTick);
    
    // 지난 프레임으로부터 경과한 틱 수 계산 후 초 단위로 변환
    long long elapsedTick = m_currentCpuTick.QuadPart - m_prevCpuTick.QuadPart;
    m_unScaledDeltaTime = static_cast<float>(elapsedTick) / static_cast<float>(m_frequency.QuadPart);

    // 다음 프레임 계산을 위해 현재 틱을 이전 틱으로 저장
    m_prevCpuTick = m_currentCpuTick;

    // 예외처리 dt가 비정상적으로 커질때를 대비한 데드존 설정
    if (m_unScaledDeltaTime > 0.1f)
    {
        m_unScaledDeltaTime = 0.1f;
    }

    if (m_bIsPaused)
    {
        m_deltaTime = 0.0f;
    }
    else
    {
        m_deltaTime = m_unScaledDeltaTime * m_timeScale;
    }

    m_unScaledGameTime += m_unScaledDeltaTime;
    m_gameTime += m_deltaTime;

    // 매 프레임 가변 시간을 누적
    m_accumulator += m_deltaTime;
    if (m_accumulator > 0.25f) m_accumulator = 0.25f;

    // FPS 계산 로직
    m_frameCount++;
    m_fpsFrameTime += m_unScaledDeltaTime;

    // 1초 누적시마다 프레임 수를 FPS로 확정하고 초기화
    if (m_fpsFrameTime >= 1.0f)
    {
        m_fps = m_frameCount;
        m_frameCount = 0;
        m_fpsFrameTime -= 1.0f;
    }
}

// 누적된 시간이 고정 프레임 시간보다 많이 남았는지 체크
bool TimeManager::AccumulateTime()
{
    return m_accumulator >= m_fixedDeltaTime;
}

// FixedUpdate 로직을 한번 실행 이후 고정 시간을 뺌
void TimeManager::ConsumeFixedTick()
{
    m_accumulator -= m_fixedDeltaTime;
}

void TimeManager::Release()
{
    // TODO :
}

