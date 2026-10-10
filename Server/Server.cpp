#include "Server.h"
#include <ws2tcpip.h>
#include <NetworkUtilities.h>
#include <chrono>

#define DEFAULT_PORT "3000"
#define MESSAGE_BUFFER_SIZE 1024

Server::Server()
{
}

Server::~Server()
{
	WSACleanup();
}

bool Server::Start() 
{
	WSADATA wsaData;

	int nIsSuccessfull =  WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nIsSuccessfull != 0)
	{
		std::cerr << "Failed to initialize winsock2" << std::endl;
		WSACleanup();
		return false;
	}

	addrinfo oHints;
	memset(&oHints, NULL, sizeof(addrinfo));

	addrinfo* pResultAddressInfo = nullptr;

	oHints.ai_family = AF_INET;
	oHints.ai_socktype = SOCK_DGRAM;
	oHints.ai_protocol = IPPROTO_UDP;
	oHints.ai_flags = AI_PASSIVE;

	nIsSuccessfull = getaddrinfo(nullptr, DEFAULT_PORT, &oHints, &pResultAddressInfo);
	if (nIsSuccessfull != 0)
	{
		std::cerr << "Failed to get address information " << std::endl;
		WSACleanup();
		return false;
	}

	addrinfo* pCurrentAddressInfo = pResultAddressInfo;
	while(pCurrentAddressInfo != nullptr)
	{
		m_oServerSocket = socket(pCurrentAddressInfo->ai_family, pCurrentAddressInfo->ai_socktype, pCurrentAddressInfo->ai_protocol);
		if (m_oServerSocket != INVALID_SOCKET)
		{
			u_long mode = 1;
			if (ioctlsocket(
				m_oServerSocket,
				FIONBIO,
				&mode
			) == SOCKET_ERROR)
			{
				std::cerr << "Error ocurred while creating non blocking socket" << std::endl;
				continue;
			}
			break;
		}

		pCurrentAddressInfo = pCurrentAddressInfo->ai_next;
	}

	if (m_oServerSocket == INVALID_SOCKET)
	{
		std::cerr << "Failed to create a socket to addrress " << pCurrentAddressInfo->ai_addr << std::endl;
		freeaddrinfo(pResultAddressInfo);
		WSACleanup();
		return false;
	}

	nIsSuccessfull = bind(m_oServerSocket, pCurrentAddressInfo->ai_addr, (int)pCurrentAddressInfo->ai_addrlen);
	if (nIsSuccessfull == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		std::cerr << "Failed to create a socket to address " << errorCode << std::endl;
		closesocket(m_oServerSocket);
		WSACleanup();
		return false;
	}

	m_strServerIP = NetworkUtilities::GetIPRepresentation(*pCurrentAddressInfo);
	freeaddrinfo(pCurrentAddressInfo);

	m_bIsServerRuning = true;
	std::cout << "Successfully started server on " << m_strServerIP << ":"<< DEFAULT_PORT << std::endl;

	Run();

	return true;
}

void Server::Run()
{
	using clock = std::chrono::steady_clock;

	auto lastTime = clock::now();
	float snapshotAccumulator = 0.0f;

	constexpr float snapshotInterval = 1.0f / 20.0f; // 20 times/sec

	while (m_bIsServerRuning)
	{
		auto currentTime = clock::now();

		float deltaTime =
			std::chrono::duration<float>(
				currentTime - lastTime
			).count();

		lastTime = currentTime;

		RecievePacket();

		snapshotAccumulator += deltaTime;
		
		if (snapshotAccumulator >= snapshotInterval)
		{
			if (m_oPlayers.size() > 0)
				UpdateWorldState();

			snapshotAccumulator -= snapshotInterval;
		}
	}
}

void Server::RecievePacket()
{
	while (true)
	{
		char messageBuffer[MESSAGE_BUFFER_SIZE];

		sockaddr_in clientAddress{};
		int clientAddressSize = sizeof(clientAddress);

		int receivedBytes = recvfrom(
			m_oServerSocket,
			messageBuffer,
			MESSAGE_BUFFER_SIZE - 1,
			0,
			reinterpret_cast<sockaddr*>(&clientAddress),
			&clientAddressSize
		);

		if (receivedBytes == SOCKET_ERROR)
		{
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK)
			{
				// Няма пакет в момента.
				break;
			}

			// Реална socket грешка.
			std::cerr << "recvfrom failed: " << error << '\n';
			return;
		}

		if (receivedBytes < sizeof(PacketHeader))
		{
			std::cerr << "Corrupted packet recieved" << std::endl;
			return;
		}

		const PacketHeader* pPacketPacket = reinterpret_cast<PacketHeader*>(messageBuffer);
		if (!pPacketPacket)
		{
			std::cerr << "An erro occured while desiriliazing a packet" << std::endl;
			return;
		}

		if (!HandlePacket(pPacketPacket->ePacketType, clientAddress, messageBuffer))
		{
			std::cerr << "An erro occured while handling a packet" << std::endl;
			return;
		}

		messageBuffer[receivedBytes] = '\0';
	}
}

bool Server::HandlePacket(const PacketTypes& ePacketType, const sockaddr_in& clientAddress, char* pPacketData)
{
	switch (ePacketType)
	{
	case Connect:
	{
		HandShakePacket oHandshakePacket;
		oHandshakePacket.isClientConnected = 1;

		SendPacket<HandShakePacket>(oHandshakePacket, clientAddress);
		break;
	}
	case CreatePlayer:
	{
		CreateNewPlayer(clientAddress);
		break;
	}

	case PlayerInput:
	{
		PlayerInputPacket* pPlayerInputPacket = reinterpret_cast<PlayerInputPacket*>(pPacketData);
		UpdatePlayer(pPlayerInputPacket);
		break;
	}

	default:
		std::cerr << "Invalid packet type recieved" << std::endl;
		return false;
	}

	return true;
}

bool Server::CreateNewPlayer(const sockaddr_in& clientAddress)
{
	if (m_oPlayers.size() + 1 > Globals::nMaxPlayers)
	{
		std::cout << "Maximum number of pleyers reached" << std::endl;
		return false;
	}

	Player player;
	player.nId = m_NextPlayerId;
	player.oPlayerAddress = clientAddress;
	player.x = rand() % Globals::nWindowWidth + 1;
	player.y = rand() % Globals::nWindowHeight + 1;

	m_NextPlayerId++;

	//TODO if player exists;																			

	m_oPlayers.push_back(player);

	std::cout << "Played with id connected to the server" << std::endl;

	WelcomePacket oWelcomePacket;
	oWelcomePacket.id = player.nId;

	SendPacket(oWelcomePacket,player.oPlayerAddress);
	return true;
}

void Server::UpdatePlayer(PlayerInputPacket* pPlayerInputPacket)
{
	for (int i = 0; i < m_oPlayers.size(); ++i)
	{
		Player oCurrentPlayer = m_oPlayers[i];

		if (oCurrentPlayer.nId != pPlayerInputPacket->id)
			continue;

		if(pPlayerInputPacket->movX != 0)
			oCurrentPlayer.x += pPlayerInputPacket->movX * pPlayerInputPacket->velocity ;

		if(pPlayerInputPacket->movY != 0)
			oCurrentPlayer.y += pPlayerInputPacket->movY * pPlayerInputPacket->velocity ;

		m_oPlayers[i] = oCurrentPlayer;
		return;
	}
}

bool Server::ShutDown()
{
	WSACleanup();
	return true;
}

bool Server::UpdateWorldState()
{
	WorldStatePacket oWorldStatePacket;
	oWorldStatePacket.playerCount = m_oPlayers.size();
	
	for (int index = 0; index < oWorldStatePacket.playerCount; ++index)
	{
		oWorldStatePacket.players[index] = m_oPlayers[index];
	}

	for (int index = 0; index < oWorldStatePacket.playerCount; ++index)
	{
		SendPacket(oWorldStatePacket, m_oPlayers[index].oPlayerAddress);
	}

	return true;
}