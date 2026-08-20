#include "Engine/Core/pch.h"
#include "NetworkManager.h"
#include "Engine/Manager/TimeManager.h"
#include "Engine/Manager/SceneManager.h"
#include "Engine/Manager/RandomManager.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"
#include "Engine/Framework/Components/Physics/BoxCollider.h"
#include "Engine/Framework/Components/Network/NetworkIdentity.h"

NetworkManager::~NetworkManager() {
	Release();
}

bool NetworkManager::Initialize() {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		std::cerr << "WSAStartup failed: " << result << std::endl;
		return false;
	}

	GUISystem::GetInstance()->RegisterPanel(this);
	return true;
}

void NetworkManager::Release() {
	GUISystem::GetInstance()->UnRegisterPanel(this);

	if (m_Role == NetRole::CLIENT && m_bConnected && m_Socket != INVALID_SOCKET) {
		ClientDisconnPacket disconnPacket;
		disconnPacket.header.type = PacketType::CLIENT_DISCONN;
		disconnPacket.header.size = sizeof(ClientDisconnPacket);
		disconnPacket.disconnectedNetID = m_MyNetID;
		SendPacket(&disconnPacket, sizeof(ClientDisconnPacket));
	}

	StopNetwork();

	WSACleanup();
	m_networkObjects.clear();
	m_ConnectedClients.clear();
}

void NetworkManager::StopNetwork() {
	m_bNetworkThreadRunning = false;

	if (m_Socket != INVALID_SOCKET) {
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}

	if (m_networkThread.joinable()) {
		m_networkThread.join();
	}

	m_Role = NetRole::NONE;
	m_bConnected = false;
	m_connRetryTimer = 0.0f;
	m_connTimeoutTimer = 0.0f;

	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		m_incomingPacketQueue.clear();
	}
}

bool NetworkManager::StartHost(int port) {
	StopNetwork();

	m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_Socket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
		return false;
	}

	u_long mode = 1;
	if (ioctlsocket(m_Socket, FIONBIO, &mode) != 0) {
		std::cerr << "ioctlsocket failed: " << WSAGetLastError() << std::endl;
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return false;
	}

	sockaddr_in localAddr;
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = INADDR_ANY;
	localAddr.sin_port = htons(port);

	if (::bind(m_Socket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
		std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return false;
	}

	m_Role = NetRole::HOST;
	m_MyNetID = 1;
	m_bConnected = true;
	uint32 newSeed = RandomManager::GetInstance()->GenerateNewSeed();
	m_ConnectedClients.clear();
	m_NextNetID = 2;

	Scene* scene = SceneManager::GetInstance()->GetActiveScene();
	if (scene) {
		for (auto* obj : scene->GetGameObjects()) {
			if (obj && obj->IsActive()) {
				NetworkIdentity* netId = obj->GetComponent<NetworkIdentity>();
				if (netId && netId->GetNetID() == 0) {
					netId->SetNetID(1);
					netId->SetLocalPlayer(true);
					break;
				}
			}
		}
	}

	std::cout << "Host started on port " << port << std::endl;

	m_bNetworkThreadRunning = true;
	m_networkThread = std::thread(&NetworkManager::NetworkThreadLoop, this);

	return true;
}

bool NetworkManager::ConnectToHost(const std::string& ip, int port) {
	StopNetwork();

	m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_Socket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
		return false;
	}

	u_long mode = 1;
	if (ioctlsocket(m_Socket, FIONBIO, &mode) != 0) {
		std::cerr << "ioctlsocket failed: " << WSAGetLastError() << std::endl;
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return false;
	}

	sockaddr_in localAddr;
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = INADDR_ANY;
	localAddr.sin_port = htons(0);

	if (::bind(m_Socket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
		std::cerr << "Client socket bind failed: " << WSAGetLastError() << std::endl;
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return false;
	}

	m_HostAddr.sin_family = AF_INET;
	m_HostAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &m_HostAddr.sin_addr);

	m_Role = NetRole::CLIENT;
	m_bConnected = false;
	m_connRetryTimer = 0.0f;
	m_connTimeoutTimer = 0.0f;

	PacketHeader connPacket;
	connPacket.type = PacketType::CLIENT_CONN_REQ;
	connPacket.size = sizeof(PacketHeader);
	SendPacket(&connPacket, sizeof(PacketHeader));

	std::cout << "Sent connection request to Host " << ip << ":" << port << std::endl;

	m_bNetworkThreadRunning = true;
	m_networkThread = std::thread(&NetworkManager::NetworkThreadLoop, this);

	return true;
}

void NetworkManager::Update(float dt) {
	if (m_Role == NetRole::NONE) return;

	ProcessIncomingPackets();

	if (m_Role == NetRole::CLIENT && !m_bConnected)
	{
		m_connTimeoutTimer += dt;
		if (m_connTimeoutTimer >= CONN_TIMEOUT)
		{
			std::cout << "[NetworkManager] Connection timed out (" << CONN_TIMEOUT << "s). Stopping retry." << std::endl;
			m_connTimeoutTimer = 0.0f;
			m_connRetryTimer = 0.0f;
			m_bConnected = false;
			m_Role = NetRole::NONE;

			if (m_onConnResultCallback)
			{
				m_onConnResultCallback(ConnResultCode::REJECTED);
			}
			return;
		}

		m_connRetryTimer += dt;
		if (m_connRetryTimer >= 0.5f)
		{
			m_connRetryTimer = 0.0f;

			PacketHeader connPacket;
			connPacket.type = PacketType::CLIENT_CONN_REQ;
			connPacket.size = sizeof(PacketHeader);
			SendPacket(&connPacket, sizeof(PacketHeader));

			std::cout << "Retrying connection to Host..." << std::endl;
		}
	}
}

void NetworkManager::FixedUpdate(float fixedDt) {
	if (m_Role == NetRole::NONE) return;

	m_currentTick++;
	TickUpdate();
}

void NetworkManager::TickUpdate() {
	if (m_Role == NetRole::CLIENT && m_bConnected) {
		QueryPerformanceCounter(&m_LastHeartbeatSentTick);

		PacketHeader hb;
		hb.type = PacketType::HEARTBEAT;
		hb.size = sizeof(PacketHeader);
		hb.tick = m_currentTick;
		SendPacket(&hb, sizeof(PacketHeader));
	}
	else if (m_Role == NetRole::HOST) {
		float currentTime = TimeManager::GetInstance()->GetRealTime();
		for (auto it = m_ConnectedClients.begin(); it != m_ConnectedClients.end();) {
			if (currentTime - it->second.lastHeartbeatTime > 30.0f) {
				uint32 deadNetID = it->first;
				std::cout << "Client (NetID: " << deadNetID << ") timed out." << std::endl;

				ClientDisconnPacket disconnPkt{};
				disconnPkt.header.type = PacketType::CLIENT_DISCONN;
				disconnPkt.header.size = sizeof(ClientDisconnPacket);
				disconnPkt.header.tick = m_currentTick;
				disconnPkt.disconnectedNetID = deadNetID;
				SendPacket(&disconnPkt, sizeof(ClientDisconnPacket));

				it = m_ConnectedClients.erase(it);
			}
			else {
				++it;
			}
		}

		m_stateBroadcastTimer += FIXED_DT;
		if (m_stateBroadcastTimer >= 0.5f) {
			m_stateBroadcastTimer = 0.0f;

			GameStateSyncPacket statePacket;
			statePacket.header.type = PacketType::GAME_STATE_SYNC;
			statePacket.header.size = sizeof(GameStateSyncPacket);
			statePacket.header.tick = m_currentTick;
			statePacket.currentGameState = GameState::PLAYING;
			statePacket.randomSeed = RandomManager::GetInstance()->GetSharedSeed();
			statePacket.gameElapsedTime = TimeManager::GetInstance()->GetGameTime();
			SendPacket(&statePacket, sizeof(GameStateSyncPacket));
		}
	}
}

void NetworkManager::OnDrawGUI()
{
	ImGui::Begin("Network Settings");

	static int port = 9000;
	static char ipAddress[64] = "127.0.0.1";

	if (m_Role == NetRole::NONE)
	{
		ImGui::InputInt("Port", &port);
		ImGui::InputText("IP Address", ipAddress, sizeof(ipAddress));

		if (ImGui::Button("Start Host"))
			StartHost(port);

		ImGui::SameLine();

		if (ImGui::Button("Connect Client"))
			ConnectToHost(ipAddress, port);

	}
	else
	{
		ImGui::Text("Role: %s", m_Role == NetRole::HOST ? "HOST" : "CLIENT");
		ImGui::Text("NetID: %u", m_MyNetID);
		ImGui::Text("Status: %s", m_bConnected ? "Connected" : "Connecting...");

		if (m_Role == NetRole::HOST)
		{
			ImGui::Text("Connected Clients: %zu", m_ConnectedClients.size());
		}

		if (ImGui::Button("DisConnect"))
		{
			Release();
			Initialize();
		}
	}

	ImGui::End();
}

void NetworkManager::SendPacket(const void* data, int size, const sockaddr_in* targetAddr) {
	if (m_Socket == INVALID_SOCKET) return;

	if (m_Role == NetRole::CLIENT) {
		sendto(m_Socket, (const char*)data, size, 0, (const sockaddr*)&m_HostAddr, sizeof(m_HostAddr));
	}
	else if (m_Role == NetRole::HOST) {
		if (targetAddr) {
			sendto(m_Socket, (const char*)data, size, 0, (const sockaddr*)targetAddr, sizeof(sockaddr_in));
		}
		else {
			for (const auto& client : m_ConnectedClients) {
				sendto(m_Socket, (const char*)data, size, 0, (const sockaddr*)&client.second.address, sizeof(sockaddr_in));
			}
		}
	}
}

void NetworkManager::SendReliablePacket(const void* data, int size, const sockaddr_in* targetAddr)
{
	PacketHeader* header = (PacketHeader*)data;
	header->tick = m_currentTick;

	for (int i = 0; i < 3; ++i)
	{
		SendPacket(data, size, targetAddr);
	}
}

void NetworkManager::RegisterNetworkObject(uint32 netID, GameObject* obj)
{
	m_networkObjects[netID] = obj;
}

void NetworkManager::UnRegisterNetworkObject(uint32 netID)
{
	m_networkObjects.erase(netID);
}

GameObject* NetworkManager::GetNetworkObject(uint32 netID)
{
	auto it = m_networkObjects.find(netID);
	if (it != m_networkObjects.end())
	{
		return it->second;
	}
	return nullptr;
}

void NetworkManager::NetworkThreadLoop() {
	char buffer[2048];
	sockaddr_in senderAddr;
	int senderAddrLen = sizeof(senderAddr);

	while (m_bNetworkThreadRunning) {
		if (m_Socket == INVALID_SOCKET) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		int bytesReceived = recvfrom(m_Socket, buffer, sizeof(buffer), 0, (sockaddr*)&senderAddr, &senderAddrLen);
		if (bytesReceived <= 0 || bytesReceived == SOCKET_ERROR) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		if (bytesReceived >= sizeof(PacketHeader)) {
			RawPacketData rawPacket;
			rawPacket.senderAddr = senderAddr;
			rawPacket.size = bytesReceived;
			rawPacket.buffer.assign(buffer, buffer + bytesReceived);

			{
				std::lock_guard<std::mutex> lock(m_queueMutex);
				m_incomingPacketQueue.push_back(std::move(rawPacket));
			}
		}
	}
}

void NetworkManager::ProcessIncomingPackets() {
	std::vector<RawPacketData> localQueue;
	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		localQueue.swap(m_incomingPacketQueue);
	}

	for (const auto& rawPkt : localQueue) {
		HandlePacket(rawPkt.buffer.data(), rawPkt.size, rawPkt.senderAddr);
	}
}

void NetworkManager::HandlePacket(const char* buffer, int size, const sockaddr_in& senderAddr)
{
	const PacketHeader* header = (const PacketHeader*)buffer;

	auto handlerIt = m_packetHandlers.find(header->type);
	if (handlerIt != m_packetHandlers.end() && handlerIt->second)
	{
		handlerIt->second(header, senderAddr);
	}

	switch (header->type) {
	case PacketType::CLIENT_CONN_REQ:
	{
		if (m_Role != NetRole::HOST) break;

		uint32 clientNetID = 0;
		for (const auto& client : m_ConnectedClients)
		{
			if (client.second.address.sin_addr.s_addr == senderAddr.sin_addr.s_addr &&
				client.second.address.sin_port == senderAddr.sin_port)
			{
				clientNetID = client.first;
				break;
			}
		}

		if (clientNetID == 0)
		{
			// 정원 초과(Max Clients) 검사
			if (m_ConnectedClients.size() >= m_maxClients)
			{
				std::cout << "[NetworkManager] Rejecting connection: Room Full (" << m_ConnectedClients.size() << "/" << m_maxClients << ")" << std::endl;
				ClientConnResPacket rejectPkt;
				rejectPkt.header.type = PacketType::CLIENT_CONN_RES;
				rejectPkt.header.size = sizeof(ClientConnResPacket);
				rejectPkt.header.tick = m_currentTick;
				rejectPkt.resultCode = ConnResultCode::ROOM_FULL;
				rejectPkt.assignedNetID = 0;

				SendPacket(&rejectPkt, sizeof(ClientConnResPacket), &senderAddr);
				break;
			}

			clientNetID = m_NextNetID++;
			NetClientInfo newClient;
			newClient.address = senderAddr;
			newClient.lastHeartbeatTime = TimeManager::GetInstance()->GetRealTime();
			newClient.assignedNetID = clientNetID;
			m_ConnectedClients[clientNetID] = newClient;

			std::cout << "New client connected. Assigned NetID: " << clientNetID << std::endl;
		}

		// 접속 성공 응답 패킷 전송
		ClientConnResPacket connResPkt;
		connResPkt.header.type = PacketType::CLIENT_CONN_RES;
		connResPkt.header.size = sizeof(ClientConnResPacket);
		connResPkt.header.tick = m_currentTick;
		connResPkt.resultCode = ConnResultCode::SUCCESS;
		connResPkt.assignedNetID = clientNetID;
		SendPacket(&connResPkt, sizeof(ClientConnResPacket), &senderAddr);

		// Welcome 패킷 전송
		WelcomePacket welcome;
		welcome.header.type = PacketType::HOST_WELCOME;
		welcome.header.size = sizeof(WelcomePacket);
		welcome.assignedNetID = clientNetID;
		welcome.randomSeed = RandomManager::GetInstance()->GetSharedSeed();

		SendPacket(&welcome, sizeof(WelcomePacket), &senderAddr);
		break;
	}

	case PacketType::CLIENT_CONN_RES:
	{
		if (m_Role != NetRole::CLIENT) break;
		const ClientConnResPacket* resPkt = (const ClientConnResPacket*)buffer;
		if (resPkt->resultCode != ConnResultCode::SUCCESS)
		{
			m_bConnected = false;
			m_Role = NetRole::NONE;
			m_connRetryTimer = 0.0f;
			m_connTimeoutTimer = 0.0f;

			std::cout << "[NetworkManager] Connection failed. ResultCode: " << static_cast<int>(resPkt->resultCode) << std::endl;
			if (m_onConnResultCallback)
			{
				m_onConnResultCallback(resPkt->resultCode);
			}
		}
		break;
	}

	case PacketType::HOST_WELCOME:
	{
		if (m_Role != NetRole::CLIENT) break;
		const WelcomePacket* welcome = (const WelcomePacket*)buffer;
		m_MyNetID = welcome->assignedNetID;
		m_bConnected = true;
		m_connRetryTimer = 0.0f;
		m_connTimeoutTimer = 0.0f;

		RandomManager::GetInstance()->SetSharedSeed(welcome->randomSeed);

		if (m_onConnResultCallback)
		{
			m_onConnResultCallback(ConnResultCode::SUCCESS);
		}

		std::cout << "Successfully connected to Host. Assigned NetID: " << m_MyNetID << std::endl;


		Scene* scene = SceneManager::GetInstance()->GetActiveScene();
		if (scene)
		{
			for (auto* obj : scene->GetGameObjects())
			{
				if (obj && obj->IsActive())
				{
					NetworkIdentity* netId = obj->GetComponent<NetworkIdentity>();
					if (netId && netId->GetNetID() == 0) {
						netId->SetNetID(m_MyNetID);
						netId->SetLocalPlayer(true);
						break;
					}
				}
			}
		}
		break;
	}

	case PacketType::HEARTBEAT:
	{
		if (m_Role == NetRole::HOST)
		{
			for (auto& client : m_ConnectedClients) {
				if (client.second.address.sin_addr.s_addr == senderAddr.sin_addr.s_addr &&
					client.second.address.sin_port == senderAddr.sin_port) {
					client.second.lastHeartbeatTime = TimeManager::GetInstance()->GetRealTime();
					break;
				}
			}
			PacketHeader pack;
			pack.type = PacketType::HEARTBEAT;
			pack.size = sizeof(PacketHeader);
			SendPacket(&pack, sizeof(PacketHeader), &senderAddr);
		}
		else if (m_Role == NetRole::CLIENT)
		{
			if (m_LastHeartbeatSentTick.QuadPart > 0) {
				LARGE_INTEGER currentTick, frequency;
				QueryPerformanceCounter(&currentTick);
				QueryPerformanceFrequency(&frequency);
				double elapsedTicks = static_cast<double>(currentTick.QuadPart - m_LastHeartbeatSentTick.QuadPart);
				double elapsedSeconds = elapsedTicks / static_cast<double>(frequency.QuadPart);

				// ms 계산 (최소 0.01ms 이상 보장)
				m_PingMs = static_cast<float>(elapsedSeconds * 1000.0);
			}
		}
		break;
	}

	case PacketType::CLIENT_DISCONN:
	{
		if (m_Role != NetRole::HOST) break;
		const ClientDisconnPacket* disconnPkt = (const ClientDisconnPacket*)buffer;
		uint32 targetNetID = disconnPkt->disconnectedNetID;

		for (auto it = m_ConnectedClients.begin(); it != m_ConnectedClients.end(); ++it) {
			if (it->first == targetNetID || 
				(it->second.address.sin_addr.s_addr == senderAddr.sin_addr.s_addr &&
				 it->second.address.sin_port == senderAddr.sin_port)) {
				
				uint32 deadNetID = it->first;
				std::cout << "Client (NetID: " << deadNetID << ") disconnected." << std::endl;
				m_ConnectedClients.erase(it);

				// 남아있는 다른 클라이언트들에게 이탈 브로드캐스트
				ClientDisconnPacket broadcastPkt{};
				broadcastPkt.header.type = PacketType::CLIENT_DISCONN;
				broadcastPkt.header.size = sizeof(ClientDisconnPacket);
				broadcastPkt.disconnectedNetID = deadNetID;
				SendPacket(&broadcastPkt, sizeof(ClientDisconnPacket));
				break;
			}
		}
		break;
	}


	default:
		break;
	}
}


