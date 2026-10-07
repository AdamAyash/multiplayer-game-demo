#pragma once

#include <WinSock2.h>
#include <string>
#include "PacketTypes.h"
#include <iostream>

enum ClientConnectionStatus
{
    Disconnected,
    Connecting,
    Connected
};

class Client final 
{
public:
	Client();
	~Client();

public:
	bool ConnectToServer(const std::string strServerIp, const std::string strServerPort);

    template<typename TPacket>
    bool SendPacket(const TPacket& packet) {
        static_assert(
            std::is_trivially_copyable_v<TPacket>,
            "Network packets must be trivially copyable"
            );

        int sentBytes = sendto(
            m_oClientSocket,
            reinterpret_cast<const char*>(&packet),
            static_cast<int>(sizeof(TPacket)),
            0,
            reinterpret_cast<const sockaddr*>(&m_oServerAddress),
            sizeof(m_oServerAddress)
        );

        if (sentBytes == SOCKET_ERROR)
        {
            std::cerr
                << "sendto failed: "
                << WSAGetLastError()
                << std::endl;

            return false;
        }

        return true;
    }

	bool ReceivePackets(PacketTypes& ePacketType, char*& pPacketData);

private:
    bool GetServerHandShake();

private:
	SOCKET m_oClientSocket = INVALID_SOCKET;
	std::string m_strServerIp;
	std::string m_strServerPort;
    sockaddr m_oServerAddress;
    ClientConnectionStatus m_eClientConnectionStatus = Disconnected;
};