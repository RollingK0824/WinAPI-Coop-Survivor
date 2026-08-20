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

using PacketHandler = std::function<void(const PacketHeader* packet, const sockaddr_in& sender)>;

class NetworkManager : public Singleton<NetworkManager>, public ISystem, public IUpdatable, public IGUIPanel{
    friend class Singleton<NetworkManager>;
public:
    virtual bool Initialize() override;
    virtual void Release() override;
    virtual void Update(float dt) override;
    virtual void FixedUpdate(float fixedDt) override;

    virtual void OnDrawGUI() override;

    bool StartHost(int port);
    bool ConnectToHost(const std::string& ip, int port);
    void StopNetwork();
    
    void SendPacket(const void* data, int size, const sockaddr_in* targetAddr = nullptr);
    void SendReliablePacket(const void* data, int size, const sockaddr_in* targetAddr = nullptr);

    void RegisterNetworkObject(uint32 netID, GameObject* obj);
    void UnRegisterNetworkObject(uint32 netID);
    GameObject* GetNetworkObject(uint32 netID);

    void RegisterPacketHandler(PacketType type, PacketHandler handler) { m_packetHandlers[type] = handler; }
    void UnregisterPacketHandler(PacketType type) { m_packetHandlers.erase(type); }
    void ClearPacketHandlers() { m_packetHandlers.clear(); }
    void ClearNetworkObjects() { m_networkObjects.clear(); }

    using ConnResultCallback = std::function<void(ConnResultCode code)>;
    void SetOnConnResultCallback(ConnResultCallback callback) { m_onConnResultCallback = callback; }

    void SetMaxClients(size_t maxClients) { m_maxClients = maxClients; }
    size_t GetMaxClients() const { return m_maxClients; }

    void SetCanJoin(bool canJoin) { m_bCanJoin = canJoin; }
    bool CanJoin() const { return m_bCanJoin; }

    NetRole GetRole() const { return m_Role; }
    uint32 GetMyNetID() const { return m_MyNetID; }
    bool IsConnected() const { return m_bConnected; }
    const std::unordered_map<uint32, NetClientInfo>& GetConnectedClients() const { return m_ConnectedClients; }

    uint32 GetCurrentTick() const { return m_currentTick; }

    float GetPing() const { return m_PingMs; }

private:
    NetworkManager() = default;
    virtual ~NetworkManager() override;

    size_t m_maxClients = 3; // 기본값 3
    ConnResultCallback m_onConnResultCallback = nullptr;


    void ProcessIncomingPackets();
    void HandlePacket(const char* buffer, int size, const sockaddr_in& senderAddr);
    void NetworkThreadLoop();
    void TickUpdate(); // 고정 Tick마다 호출: 패킷 전송, 타임아웃 검사 등

private:
    static constexpr int   FIXED_TICK_RATE = 60;
    static constexpr float FIXED_DT        = 1.0f / FIXED_TICK_RATE;

    NetRole m_Role = NetRole::NONE;
    SOCKET m_Socket = INVALID_SOCKET;
    sockaddr_in m_HostAddr{};

    std::thread m_networkThread;
    std::mutex m_queueMutex;
    std::atomic<bool> m_bNetworkThreadRunning = false;
    std::vector<RawPacketData> m_incomingPacketQueue;

    std::unordered_map<uint32, GameObject*> m_networkObjects;
    std::unordered_map<uint32, NetClientInfo> m_ConnectedClients; 
    std::unordered_map<PacketType, PacketHandler> m_packetHandlers;

    uint32 m_MyNetID = 0;
    bool m_bConnected = false;
    bool m_bCanJoin = true;

    // Fixed Tick
    uint32 m_currentTick     = 0;

    uint32 m_NextNetID = 1000; 

    float m_PingMs = 0.0f;
    LARGE_INTEGER m_LastHeartbeatSentTick{};

    float m_stateBroadcastTimer = 0.0f;
    float m_connRetryTimer = 0.0f;
    float m_connTimeoutTimer = 0.0f;
    static constexpr float CONN_TIMEOUT = 3.0f;
};
