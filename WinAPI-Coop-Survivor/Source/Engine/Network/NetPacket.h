#pragma once

enum class PacketType : uint8 {
	NONE = 0,
	CLIENT_CONN_REQ,  // Client -> Host connection request
	HOST_WELCOME,     // Host -> Client welcome
	HEARTBEAT,        // 핑/연결 유지
	PLAYER_INPUT,     // Client -> Host player input
	ENTITY_STATE_SYNC,// Host -> Client state sync
	GAME_STATE_SYNC,  // Room/Game State Sync
	CLIENT_DISCONN    // Disconnect
};

#pragma pack(push, 1)

struct PacketHeader {
	PacketType type;
	uint16 size;
	uint32 sequenceNumber;
};

// WelcomePacket
struct WelcomePacket {
	PacketHeader header;
	uint32 assignedNetID; // assigned client NetID
	uint32 randomSeed;
};

// PlayerInputPacket
struct PlayerInputPacket {
	PacketHeader header;
	uint8 netID;
	float posX;
	float posY;
	float velX;
	float velY;
	float angle;
};

// EntitySyncData
struct EntitySyncData {
	uint8 netID;
	float posX;
	float posY;
	float velX;
	float velY;
	float angle;
};

// Heartbeat & Ping 겸용 Packet
struct HeartbeatPacket
{
	PacketHeader header;
	uint8 clientTime;
};

// EntityStateSyncPacket
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

#pragma pack(pop)
