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

	if (m_Socket != INVALID_SOCKET) {
		// 클라이언트인 경우 종료 알림 전송
		if (m_Role == NetRole::CLIENT && m_bConnected) {
			PacketHeader disconnPacket;
			disconnPacket.type = PacketType::CLIENT_DISCONN;
			disconnPacket.size = sizeof(PacketHeader);
			SendPacket(&disconnPacket, sizeof(PacketHeader));
		}

		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
	WSACleanup();
	m_Role = NetRole::NONE;
	m_bConnected = false;
	m_ConnectedClients.clear();
	m_InterpolationMap.clear();
}

bool NetworkManager::StartHost(int port) {
	m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_Socket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
		return false;
	}

	// 논블로킹 설정
	u_long mode = 1;
	if (ioctlsocket(m_Socket, FIONBIO, &mode) != 0) {
		std::cerr << "ioctlsocket failed: " << WSAGetLastError() << std::endl;
		closesocket(m_Socket);
		return false;
	}

	sockaddr_in localAddr;
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = INADDR_ANY;
	localAddr.sin_port = htons(port);

	if (::bind(m_Socket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
		std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
		closesocket(m_Socket);
		return false;
	}

	m_Role = NetRole::HOST;
	m_MyNetID = 1; // Host의 NetID는 항상 1로 지정
	m_bConnected = true;
	uint32 newSeed = RandomManager::GetInstance()->GenerateNewSeed();
	m_ConnectedClients.clear();
	m_InterpolationMap.clear();
	m_NextNetID = 1000;

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
	return true;
}

bool NetworkManager::ConnectToHost(const std::string& ip, int port) {
	m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_Socket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
		return false;
	}

	// 논블로킹 설정
	u_long mode = 1;
	if (ioctlsocket(m_Socket, FIONBIO, &mode) != 0) {
		std::cerr << "ioctlsocket failed: " << WSAGetLastError() << std::endl;
		closesocket(m_Socket);
		return false;
	}

	// 클라이언트 소켓 바인딩 (임의 포트)
	sockaddr_in localAddr;
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = INADDR_ANY;
	localAddr.sin_port = htons(0);

	if (::bind(m_Socket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
		std::cerr << "Client socket bind failed: " << WSAGetLastError() << std::endl;
		closesocket(m_Socket);
		return false;
	}

	m_HostAddr.sin_family = AF_INET;
	m_HostAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &m_HostAddr.sin_addr);

	m_Role = NetRole::CLIENT;
	m_bConnected = false;
	m_connRetryTimer = 0.0f;
	m_InterpolationMap.clear();

	// Host에 접속 요청 발송
	PacketHeader connPacket;
	connPacket.type = PacketType::CLIENT_CONN_REQ;
	connPacket.size = sizeof(PacketHeader);
	SendPacket(&connPacket, sizeof(PacketHeader));

	std::cout << "Sent connection request to Host " << ip << ":" << port << std::endl;
	return true;
}

void NetworkManager::Update(float dt) {
	if (m_Role == NetRole::NONE) return;

	// 1. 패킷 수신 및 처리
	ProcessIncomingPackets();

	if (m_Role == NetRole::CLIENT && !m_bConnected)
	{
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

	// 2. 보간 경과 시간 업데이트
	for (auto& pair : m_InterpolationMap) {
		pair.second.elapsed += dt;
	}

	// 3. 60Hz 주기로 패킷 전송 (Heartbeat 포함)
	m_SendTimer += dt;
	if (m_SendTimer >= m_SendInterval) {
		m_SendTimer = 0.0f;

		if (m_Role == NetRole::CLIENT && m_bConnected) {
			// Heartbeat 전송
			QueryPerformanceCounter(&m_LastHeartbeatSentTick);

			PacketHeader hb;
			hb.type = PacketType::HEARTBEAT;
			hb.size = sizeof(PacketHeader);
			SendPacket(&hb, sizeof(PacketHeader));
		}
		else if (m_Role == NetRole::HOST) {
			// 접속이 오래 끊긴 클라이언트 타임아웃 검사 (5초 간 무반응 시 제거)
			float currentTime = TimeManager::GetInstance()->GetRealTime();
			for (auto it = m_ConnectedClients.begin(); it != m_ConnectedClients.end();) {
				if (currentTime - it->second.lastHeartbeatTime > 5.0f) {
					std::cout << "Client (NetID: " << it->first << ") timed out." << std::endl;
					it = m_ConnectedClients.erase(it);
				}
				else {
					++it;
				}
			}

			m_stateBroadcastTimer += dt;
			if (m_stateBroadcastTimer >= 0.5f) {
				m_stateBroadcastTimer = 0.0f;

				GameStateSyncPacket statePacket;
				statePacket.header.type = PacketType::GAME_STATE_SYNC;
				statePacket.header.size = sizeof(GameStateSyncPacket);
				statePacket.currentGameState = GameState::PLAYING; // 현재 게임 상태
				statePacket.randomSeed = RandomManager::GetInstance()->GetSharedSeed();                    // 공유할 Seed
				statePacket.gameElapsedTime = TimeManager::GetInstance()->GetGameTime();
				// 연결된 모든 클라이언트에게 전송
				SendPacket(&statePacket, sizeof(GameStateSyncPacket));
			}

			// 60Hz 주기로 모든 클라이언트에게 상태 패킷 브로드캐스트
			Scene* scene = SceneManager::GetInstance()->GetActiveScene();
			if (scene) {
				EntityStateSyncPacket syncPacket;
				syncPacket.header.type = PacketType::ENTITY_STATE_SYNC;
				syncPacket.header.size = sizeof(EntityStateSyncPacket);
				syncPacket.entityCount = 0;

				for (auto* obj : scene->GetGameObjects()) {
					if (obj && obj->IsActive()) {
						NetworkIdentity* netIdComp = obj->GetComponent<NetworkIdentity>();
						if (netIdComp && netIdComp->GetNetID() > 0) {
							int idx = syncPacket.entityCount;
							if (idx >= 32) break; // 최대 32개 제한

							syncPacket.entities[idx].netID = netIdComp->GetNetID();

							ColliderComponent* pCollider = obj->GetComponent<ColliderComponent>();
							if (pCollider && b2Body_IsValid(pCollider->GetBodyId())) {
								b2Vec2 pos = b2Body_GetPosition(pCollider->GetBodyId());
								b2Vec2 vel = b2Body_GetLinearVelocity(pCollider->GetBodyId());
								float angle = b2Rot_GetAngle(b2Body_GetRotation(pCollider->GetBodyId()));

								syncPacket.entities[idx].posX = MeterToPixel(pos.x);
								syncPacket.entities[idx].posY = MeterToPixel(pos.y);
								syncPacket.entities[idx].velX = MeterToPixel(vel.x);
								syncPacket.entities[idx].velY = MeterToPixel(vel.y);
								syncPacket.entities[idx].angle = angle;
							}
							else {
								TransformComponent* transform = &obj->transform;
								syncPacket.entities[idx].posX = transform->GetPosition().x;
								syncPacket.entities[idx].posY = transform->GetPosition().y;
								syncPacket.entities[idx].velX = 0;
								syncPacket.entities[idx].velY = 0;
								syncPacket.entities[idx].angle = transform->GetRotation().angle;
							}
							syncPacket.entityCount++;
						}
					}
				}
				if (syncPacket.entityCount > 0) {
					int packetSize = sizeof(PacketHeader) + sizeof(int) + sizeof(EntitySyncData) * syncPacket.entityCount;
					SendPacket(&syncPacket, packetSize);
				}
			}
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
		// Client는 항상 Host에게 보냄
		sendto(m_Socket, (const char*)data, size, 0, (const sockaddr*)&m_HostAddr, sizeof(m_HostAddr));
	}
	else if (m_Role == NetRole::HOST) {
		if (targetAddr) {
			sendto(m_Socket, (const char*)data, size, 0, (const sockaddr*)targetAddr, sizeof(sockaddr_in));
		}
		else {
			// 브로드캐스트 (모든 클라이언트에게 전송)
			for (const auto& client : m_ConnectedClients) {
				sendto(m_Socket, (const char*)data, size, 0, (const sockaddr*)&client.second.address, sizeof(sockaddr_in));
			}
		}
	}
}

void NetworkManager::SendReliablePacket(const void* data, int size, const sockaddr_in* targetAddr)
{
	PacketHeader* header = (PacketHeader*)data;
	header->sequenceNumber = m_txSequenceNumber++;

	for (int i = 0; i < 3; ++i)
	{
		SendPacket(data, size, targetAddr);
	}
}

void NetworkManager::ProcessIncomingPackets() {
	char buffer[2048];
	sockaddr_in senderAddr;
	int senderAddrLen = sizeof(senderAddr);

	while (true) {
		int bytesReceived = recvfrom(m_Socket, buffer, sizeof(buffer), 0, (sockaddr*)&senderAddr, &senderAddrLen);
		if (bytesReceived == SOCKET_ERROR) {
			int errorCode = WSAGetLastError();
			if (errorCode != WSAEWOULDBLOCK) {
				std::cerr << "recvfrom failed with error: " << errorCode << std::endl;
			}
			break; // 더 이상 읽을 데이터가 없음 (또는 에러)
		}

		if (bytesReceived >= sizeof(PacketHeader)) {
			HandlePacket(buffer, bytesReceived, senderAddr);
		}
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

		// 이미 등록된 클라이언트인지 검사 (동일한 Address 포트 비교)
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

		// 신규 클라이언트라면 NetID 생성 및 등록
		if (clientNetID == 0)
		{
			clientNetID = m_NextNetID++;
			NetClientInfo newClient;
			newClient.address = senderAddr;
			newClient.lastHeartbeatTime = TimeManager::GetInstance()->GetRealTime();
			newClient.assignedNetID = clientNetID;
			m_ConnectedClients[clientNetID] = newClient;

			std::cout << "New client connected. Assigned NetID: " << clientNetID << std::endl;
		}

		// Welcome 패킷 전송
		WelcomePacket welcome;
		welcome.header.type = PacketType::HOST_WELCOME;
		welcome.header.size = sizeof(WelcomePacket);
		welcome.assignedNetID = clientNetID;
		welcome.randomSeed = RandomManager::GetInstance()->GetSharedSeed();

		SendPacket(&welcome, sizeof(WelcomePacket), &senderAddr);
		break;
	}

	case PacketType::HOST_WELCOME:
	{
		if (m_Role != NetRole::CLIENT) break;
		const WelcomePacket* welcome = (const WelcomePacket*)buffer;
		m_MyNetID = welcome->assignedNetID;
		m_bConnected = true;

		RandomManager::GetInstance()->SetSharedSeed(welcome->randomSeed);

		std::cout << "Successfully connected to Host. Assigned NetID: " << m_MyNetID << std::endl;

		// Bind player controllers when WELCOME packet arrives with assigned NetID
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
		for (auto it = m_ConnectedClients.begin(); it != m_ConnectedClients.end(); ++it) {
			if (it->second.address.sin_addr.s_addr == senderAddr.sin_addr.s_addr &&
				it->second.address.sin_port == senderAddr.sin_port) {
				std::cout << "Client (NetID: " << it->first << ") disconnected." << std::endl;
				m_ConnectedClients.erase(it);
				break;
			}
		}
		break;
	}

	case PacketType::ENTITY_STATE_SYNC:
	{
		if (m_Role != NetRole::CLIENT) break;
		const EntityStateSyncPacket* syncPacket = (const EntityStateSyncPacket*)buffer;
		for (int i = 0; i < syncPacket->entityCount; ++i)
		{
			const EntitySyncData& data = syncPacket->entities[i];
			if (data.netID != m_MyNetID)
			{
				UpdateInterpolationTarget(data.netID, data.posX, data.posY, data.angle);
			}
		}
		break;
	}
	case PacketType::GAME_STATE_SYNC:
	{
		if (m_Role != NetRole::CLIENT) break;
		const GameStateSyncPacket* packet = (const GameStateSyncPacket*)buffer;
		if (RandomManager::GetInstance()->GetSharedSeed() != packet->randomSeed)
		{
			RandomManager::GetInstance()->SetSharedSeed(packet->randomSeed);
		}
		break;
	}
	default:
		break;
	}
}

void NetworkManager::UpdateInterpolationTarget(unsigned int netID, float targetX, float targetY, float targetAngle) {
	auto& data = m_InterpolationMap[netID];

	float currentX = data.targetX;
	float currentY = data.targetY;
	float currentAngle = data.targetAngle;

	if (data.elapsed > 0.0f && data.elapsed < data.duration) {
		float t = data.elapsed / data.duration;
		currentX = data.startX + (data.targetX - data.startX) * t;
		currentY = data.startY + (data.targetY - data.startY) * t;
		currentAngle = data.startAngle + (data.targetAngle - data.startAngle) * t;
	}
	else if (data.elapsed == 0.0f && data.startX == 0.0f && data.startY == 0.0f) {
		currentX = targetX;
		currentY = targetY;
		currentAngle = targetAngle;
	}

	data.startX = currentX;
	data.startY = currentY;
	data.startAngle = currentAngle;
	data.targetX = targetX;
	data.targetY = targetY;
	data.targetAngle = targetAngle;
	data.elapsed = 0.0f;
	data.duration = m_SendInterval;
}

bool NetworkManager::GetInterpolatedPosition(unsigned int netID, float& outX, float& outY, float& outAngle) {
	auto it = m_InterpolationMap.find(netID);
	if (it == m_InterpolationMap.end()) {
		return false;
	}

	const auto& data = it->second;
	float t = data.elapsed / data.duration;
	if (t > 1.0f) t = 1.0f;
	if (t < 0.0f) t = 0.0f;

	outX = data.startX + (data.targetX - data.startX) * t;
	outY = data.startY + (data.targetY - data.startY) * t;
	outAngle = data.startAngle + (data.targetAngle - data.startAngle) * t;
	return true;
}
