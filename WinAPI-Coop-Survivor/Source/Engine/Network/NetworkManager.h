#pragma once
#include "Engine/Core/Singleton.h"
#include "Engine/Network/NetPacket.h"
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IUpdatable.h"
#include "Engine/Manager/GUISystem.h"

enum class NetRole {
    NONE,
    HOST,
    CLIENT
};

struct NetClientInfo {
    sockaddr_in address{};
    float lastHeartbeatTime = 0.0f;
    unsigned int assignedNetID = 0;
};

struct InterpolationData {
    float startX = 0.0f;
    float startY = 0.0f;
    float targetX = 0.0f;
    float targetY = 0.0f;
    float startAngle = 0.0f;
    float targetAngle = 0.0f;
    float elapsed = 0.0f;
    float duration = 0.0166f; // 30Hz -> 33ms
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

    void RegisterPacketHandler(PacketType type, PacketHandler handler) { m_packetHandlers[type] = handler; }
    void UnregisterPacketHandler(PacketType type) { m_packetHandlers.erase(type); }

    NetRole GetRole() const { return m_Role; }
    unsigned int GetMyNetID() const { return m_MyNetID; }
    bool IsConnected() const { return m_bConnected; }

    bool GetInterpolatedPosition(unsigned int netID, float& outX, float& outY, float& outAngle);
    void UpdateInterpolationTarget(unsigned int netID, float targetX, float targetY, float targetAngle);

    float GetPing() const { return m_PingMs; }

private:
    NetworkManager() = default;
    virtual ~NetworkManager() override;

    void ProcessIncomingPackets();
    void HandlePacket(const char* buffer, int size, const sockaddr_in& senderAddr);

private:
    NetRole m_Role = NetRole::NONE;
    SOCKET m_Socket = INVALID_SOCKET;
    sockaddr_in m_HostAddr{};

    std::unordered_map<unsigned int, NetClientInfo> m_ConnectedClients; 
    std::unordered_map<unsigned int, InterpolationData> m_InterpolationMap; 
    std::unordered_map<PacketType, PacketHandler> m_packetHandlers;

    unsigned int m_MyNetID = 0;
    bool m_bConnected = false;

    float m_SendTimer = 0.0f;
    const float m_SendInterval = 0.0166f; 
    unsigned int m_NextNetID = 1000; 

    float m_PingMs = 0.0f;
    double m_LastHeartbeatSentMs = 0.0;
    LARGE_INTEGER  m_LastHeartbeatSentTick{};

    uint32 m_txSequenceNumber = 1;
    uint32 m_rxLastSequenceNumber = 0;
    float m_stateBroadcastTimer = 0.0f;

    float m_connRetryTimer = 0.0f;
};
