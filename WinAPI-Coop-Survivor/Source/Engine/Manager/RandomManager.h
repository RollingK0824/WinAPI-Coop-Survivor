#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"

class RandomManager : public Singleton<RandomManager>, public ISystem
{
	friend class Singleton<RandomManager>;
public:
	virtual bool Initialize() override;
	virtual void Release() override {};

	uint32 GenerateNewSeed();
	void SetSharedSeed(uint32 seed);
	uint32 GetSharedSeed() const { return m_sharedSeed; }

	int32 GetSharedRandomInt(int32 min, int32 max);
	float GetSharedRandomFloat(float min, float max);

	int32 GetLocalRandomInt(int32 min, int32 max);
	float GetLocalRandomFloat(float min, float max);

private:
	RandomManager() = default;
	virtual ~RandomManager() = default;

private:
	std::mt19937 m_sharedEngine;
	std::mt19937 m_localEngine;

	uint32 m_sharedSeed = 0;
};

