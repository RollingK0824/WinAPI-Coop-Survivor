#pragma once

enum class PacketType : uint8 {
	NONE = 0,
	CLIENT_CONN_REQ,  // Client -> Host connection request
	HOST_WELCOME,     // Host -> Client welcome
	HEARTBEAT,        // 핑/연결 유지
	PLAYER_INPUT,     // Client -> Host player input
	ENTITY_STATE_SYNC,// Host -> Client state sync
	GAME_STATE_SYNC,  // Room/Game State Sync
	CLIENT_DISCONN,    // Disconnect
	CLIENT_READY_REQ,
	GAME_START_SIGNAL,
	MONSTER_SNAPSHOT,
	MONSTER_KILL,
	SKILL_FIRE,
	TEAM_EXP_SYNC,
	CLIENT_CONN_RES,
	SKILL_SLOT_SYNC,
	PARTY_HP_SYNC,
	SKILL_CHOICE_COMPLETE,
	SIMULATION_RESUME_SIGNAL
};

enum class ConnResultCode : uint8 {
	SUCCESS = 0,
	ROOM_FULL,
	INVALID_VERSION,
	REJECTED
};

#pragma pack(push, 1)
struct PacketHeader {
	PacketType type;
	uint16 size;
	uint32 tick; // 패킷 생성 시점의 고정 Tick 카운터
};

struct SkillChoiceCompletePacket {
	PacketHeader header;
	uint32 playerNetID;
	uint32 chosenSkillID;
	uint8 choiceType;
};

struct SimulationResumeSignalPacket {
	PacketHeader header;
};


struct ClientConnResPacket {
	PacketHeader header;
	ConnResultCode resultCode;
	uint32 assignedNetID;
};

struct SkillSlotSyncPacket {
	PacketHeader header;
	uint8 playerNetID;
	uint8 slotIndex;
	uint32 skillID;
	uint8 level;
};


struct PartyHPSyncPacket {
	PacketHeader header;
	uint8 playerNetID;
	float currentHP;
	float maxHP;
};


struct WelcomePacket {
	PacketHeader header;
	uint32 assignedNetID;
	uint32 randomSeed;
};

struct PlayerInputPacket {
	PacketHeader header;
	uint8 netID;
	Vector2 pos;
	Vector2 vel;
	float angle;
};

struct EntitySyncData {
	uint8 netID;
	Vector2 pos;
	Vector2 vel;
	float angle;
	float hp;
};

struct HeartbeatPacket
{
	PacketHeader header;
	uint8 clientTime;
};

struct EntityStateSyncPacket {
	PacketHeader header;
	int32 entityCount;
	EntitySyncData entities[32];
};

enum class GameState : uint8 { LOBBY, PLAYING, GAME_OVER };

struct GameStateSyncPacket
{
	PacketHeader header;
	GameState currentGameState;
	uint32 randomSeed;
	float gameElapsedTime;
};

struct ClientDisconnPacket
{
	PacketHeader header;
	uint32 disconnectedNetID;
};

struct ClientReadyReqPacket
{
	PacketHeader header;
	uint32 netId;
	bool isReady;
};

struct GameStartSignalPacket
{
	PacketHeader header;
	uint32 randomSeed;
	float countdown;
};

struct MonsterSnapshotData
{
	uint16 monsterNetID;
	uint32 monsterAssetID;
	Vector2 pos;
};

struct MonsterSnapshotPacket
{
	PacketHeader header;
	uint16 monsterCount;
	MonsterSnapshotData monsters[1];
};

struct MonsterKillPacket
{
	PacketHeader header;
	uint16 monsterNetID;
	Vector2 dropItemPos;
};

struct SkillFirePacket
{
	PacketHeader header;
	uint8 playerNetID;
	uint32 skillID;
	Vector2 spawnPos;
	Vector2 dir;
};

struct TeamExpSyncPacket
{
	PacketHeader header;
	int32 teamLevel;
	float teamExp;
	float teamMaxExp;
};
#pragma pack(pop)
