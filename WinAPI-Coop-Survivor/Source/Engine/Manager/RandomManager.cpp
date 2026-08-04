#include "Engine/Core/pch.h"
#include "RandomManager.h"

bool RandomManager::Initialize()
{
	std::random_device rd;
	m_localEngine.seed(rd());
	GenerateNewSeed();

	return true;
}

uint32 RandomManager::GenerateNewSeed()
{
	std::random_device rd;
	m_sharedSeed = rd();
	m_sharedEngine.seed(m_sharedSeed);
	std::cout<< "[RandomManager] New Shared Seed Generated: " << m_sharedSeed << std::endl;

	return m_sharedSeed;
}

void RandomManager::SetSharedSeed(uint32 seed)
{
	m_sharedSeed = seed;
	m_sharedEngine.seed(m_sharedSeed);
	std::cout << "[RandomManager] Shared Seed Synchronized from Host: " << m_sharedSeed << std::endl;
}

int32 RandomManager::GetSharedRandomInt(int32 min, int32 max)
{
	if (min > max)std::swap(min, max);
	std::uniform_int_distribution<int32>dist(min, max);
	return dist(m_sharedEngine);
}

float RandomManager::GetSharedRandomFloat(float min, float max)
{
	if (min > max) std::swap(min, max);
	std::uniform_real_distribution<float> dist(min, max);
	return dist(m_sharedEngine);
}

int32 RandomManager::GetLocalRandomInt(int32 min, int32 max)
{
	if (min > max) std::swap(min, max);
	std::uniform_int_distribution<int32> dist(min, max);
	return dist(m_localEngine);
}

float RandomManager::GetLocalRandomFloat(float min, float max)
{
	if (min > max) std::swap(min, max);
	std::uniform_real_distribution<float> dist(min, max);
	return dist(m_localEngine);
}

