#pragma once
#include "Engine/Core/pch.h"

struct OnSkillSlotChangedEvent {
	uint8 playerNetID = 0;
	uint8 slotIndex = 0;
	uint32 skillID = 0;
	uint8 level = 1;
};


struct OnPartyHPChangedEvent {
	uint8 playerNetID = 0;
	float currentHP = 100.0f;
	float maxHP = 100.0f;
};

struct OnTeamLevelUpEvent {
	int32 teamLevel = 1;
};

struct OnPlayerDeathEvent {
	uint8 playerNetID = 0;
	Vector2 deathPos = { 0.0f, 0.0f };
};

struct OnPlayerRespawnEvent {
	uint8 playerNetID = 0;
	Vector2 respawnPos = { 0.0f, 0.0f };
};
