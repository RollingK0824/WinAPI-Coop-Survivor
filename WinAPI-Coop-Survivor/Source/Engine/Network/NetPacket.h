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
	MONSTER_KILL
};

#pragma pack(push, 1)
struct PacketHeader {
	PacketType type;
	uint16 size;
	uint32 sequenceNumber;
};

struct WelcomePacket {
	PacketHeader header;
	uint32 assignedNetID;
	uint32 randomSeed;
};

struct PlayerInputPacket {
	PacketHeader header;
	uint8 netID;
	float posX;
	float posY;
	float velX;
	float velY;
	float angle;
};

struct EntitySyncData {
	uint8 netID;
	float posX;
	float posY;
	float velX;
	float velY;
	float angle;
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
	float posX;
	float posY;
};

struct MonsterSnapshotPacket
{
	PacketHeader header;
	uint32 timestamp;
	uint16 monsterCount;
	MonsterSnapshotData monsters[1];
};

struct MonsterKillPacket
{
	PacketHeader header;
	uint16 monsterNetID;
	float dropItemPosX;
	float dropItemPosY;
};
#pragma pack(pop)
