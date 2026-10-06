#include "Client.h"
#include <ws2tcpip.h>

#define MESSAGE_BUFFER_SIZE 1024

Client::Client()
{
}

Client::~Client()
{
}

bool Client::ConnectToServer(const std::string strServerIp, const std::string strServerPort)
{

	if (m_oClientSocket != INVALID_SOCKET)
		return true;

	m_strServerIp = strServerIp;
	m_strServerPort = strServerPort;

	WSADATA wsaData;

	int nIsSuccessfull = WSAStartup(MAKEWORD(2, 2), &wsaData);
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

	nIsSuccessfull = getaddrinfo(m_strServerIp.c_str(), m_strServerPort.c_str(), &oHints, &pResultAddressInfo);
	if (nIsSuccessfull != 0)
	{
		std::cerr << "Failed to get address information " << std::endl;
		WSACleanup();
		return false;
	}

	addrinfo* pCurrentAddressInfo = pResultAddressInfo;
	while (pCurrentAddressInfo != nullptr)
	{
		m_oClientSocket = socket(pCurrentAddressInfo->ai_family, pCurrentAddressInfo->ai_socktype, pCurrentAddressInfo->ai_protocol);
		if (m_oClientSocket != INVALID_SOCKET)
		{
			u_long mode = 1;
			if (ioctlsocket(
				m_oClientSocket,
				FIONBIO,
				&mode
			) == SOCKET_ERROR)
			{
				std::cerr << "Error ocurred while creating non blocking socket" << std::endl;
			}
			break;
		}

		pCurrentAddressInfo = pCurrentAddressInfo->ai_next;
	}

	if (m_oClientSocket == INVALID_SOCKET)
	{
		std::cerr << "Failed to create a socket to addrress " << pCurrentAddressInfo->ai_addr << std::endl;
		freeaddrinfo(pCurrentAddressInfo);
		WSACleanup();
		return false;
	}

	m_oServerAddress = *pCurrentAddressInfo->ai_addr;
	freeaddrinfo(pCurrentAddressInfo);

	ConnectPacket oConnectPacket;
	if (!SendPacket<ConnectPacket>(oConnectPacket))
	{
		std::cerr << "An error ocurred while connection to the server" << std::endl;
		return false;
	}

	return true;
}

bool Client::ReceivePackets(PacketTypes& ePacketType, char*& pPacketData)
{
	char messageBuffer[MESSAGE_BUFFER_SIZE];

	sockaddr_in clientAddress{};
	int clientAddressSize = sizeof(clientAddress);

	int receivedBytes = recvfrom(
		m_oClientSocket,
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
			return false;
		}

		// Реална socket грешка.
		std::cerr << "recvfrom failed: " << error << '\n';
		return false;
	}

	if (receivedBytes < sizeof(PacketHeader))
	{
		std::cerr << "Corrupted packet recieved" << std::endl;
		return false;
	}

	const PacketHeader* pPacketHeader = reinterpret_cast<PacketHeader*>(messageBuffer);
	if (!pPacketHeader)
	{
		std::cerr << "An erro occured while desiriliazing a packet" << std::endl;
		return false;
	}

	pPacketData = new char[receivedBytes];

	ePacketType = pPacketHeader->ePacketType;
	memcpy(pPacketData, messageBuffer, receivedBytes);

	return true;
}