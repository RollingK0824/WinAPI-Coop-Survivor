#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Network/NetPacket.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IUpdatable.h"
#include "Engine/Manager/GUISystem.h"

class GameObject;

enum class NetRole {
    NONE,
    HOST,
    CLIENT
};

struct NetClientInfo {
    sockaddr_in address{};
    float lastHeartbeatTime = 0.0f;
    uint32 assignedNetID = 0;
};

struct RawPacketData {
    sockaddr_in senderAddr{};
    int size = 0;
    std::vector<char> buffer;
};

struct InterpolationData {
    Vector2 startPos{ 0.0f,0.0f };
    Vector2 targetPos{ 0.0f,0.0f };
    float elapsed = 0.0f;
	float duration = 0.0166f;   // Default(Player 60Hz : 1/60(0.0166f), Monster 15Hz : 1/15(0.066f))
};

using PacketHandler = std::function<void(const PacketHeader* packet, const sockaddr_in& sender)>;

class NetworkManager : public Singleton<NetworkManager>, public ISystem, public IUpdatable, public IGUIPanel{
    friend class Singleton<NetworkManager>;
public:
    virtual bool Initialize() override;
    virtual void Release() override;
    virtual void Update(float dt) override;

    virtual void OnDrawGUI() override;

    bool StartHost(int port);
    bool ConnectToHost(const std::string& ip, int port);
    
    void SendPacket(const void* data, int size, const sockaddr_in* targetAddr = nullptr);

    void SendReliablePacket(const void* data, int size, const sockaddr_in* targetAddr = nullptr);

    void RegisterNetworkObject(uint32 netID, GameObject* obj);
    void UnRegisterNetworkObject(uint32 netID);
    GameObject* GetNetworkObject(uint32 netID);

    void RegisterPacketHandler(PacketType type, PacketHandler handler) { m_packetHandlers[type] = handler; }
    void UnregisterPacketHandler(PacketType type) { m_packetHandlers.erase(type); }

    NetRole GetRole() const { return m_Role; }
    uint32 GetMyNetID() const { return m_MyNetID; }
    bool IsConnected() const { return m_bConnected; }
    const std::unordered_map<uint32, NetClientInfo>& GetConnectedClients() const { return m_ConnectedClients; }

    bool GetInterpolatedPosition(uint32 netID, Vector2& outPos);
    bool GetInterpolatedPosition(uint32 netID, float& outX, float& outY, float& outAngle);
    void UpdateInterpolationTarget(uint32 netID, const Vector2& targetPos, float duration = 0.0166f);
    void UpdateInterpolationTarget(uint32 netID, float targetX, float targetY, float targetAngle, float duration = 0.0166f);
    void RemoveInterpolation(uint32 netID);

    float GetPing() const { return m_PingMs; }

private:
    NetworkManager() = default;
    virtual ~NetworkManager() override;

    void ProcessIncomingPackets();
    void HandlePacket(const char* buffer, int size, const sockaddr_in& senderAddr);
    void NetworkThreadLoop();

private:
    NetRole m_Role = NetRole::NONE;
    SOCKET m_Socket = INVALID_SOCKET;
    sockaddr_in m_HostAddr{};

    std::thread m_networkThread;
    std::mutex m_queueMutex;
    std::atomic<bool> m_bNetworkThreadRunning = false;
    std::vector<RawPacketData> m_incomingPacketQueue;

    std::unordered_map<uint32, GameObject*> m_networkObjects;
    std::unordered_map<uint32, NetClientInfo> m_ConnectedClients; 
    std::unordered_map<uint32, InterpolationData> m_InterpolationMap; 
    std::unordered_map<PacketType, PacketHandler> m_packetHandlers;

    uint32 m_MyNetID = 0;
    bool m_bConnected = false;

    float m_SendTimer = 0.0f;
    const float m_SendInterval = 0.0166f; 
    uint32 m_NextNetID = 1000; 

    float m_PingMs = 0.0f;
    double m_LastHeartbeatSentMs = 0.0;
    LARGE_INTEGER  m_LastHeartbeatSentTick{};

    uint32 m_txSequenceNumber = 1;
    uint32 m_rxLastSequenceNumber = 0;
    float m_stateBroadcastTimer = 0.0f;

    float m_connRetryTimer = 0.0f;
};
