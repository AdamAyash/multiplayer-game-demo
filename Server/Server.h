#pragma once

#include <WinSock2.h>
#include <string>
#include <PacketTypes.h>
#include <vector>
#include "Player.h"
#include <iostream>

class Server final
{
public:
	Server();
	~Server();

public:
	bool Start();

private:
	void Run();
	void RecievePacket();

	template<typename TPacket>
	bool SendPacket(const TPacket& packet,const sockaddr_in& clientAddress) {
        static_assert(
            std::is_trivially_copyable_v<TPacket>,
            "Network packets must be trivially copyable"
            );

        int sentBytes = sendto(
            m_oServerSocket,
            reinterpret_cast<const char*>(&packet),
            static_cast<int>(sizeof(TPacket)),
            0,
            reinterpret_cast<const sockaddr*>(&clientAddress),
            sizeof(clientAddress)
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

	bool HandlePacket(const PacketTypes& ePacketType, const sockaddr_in& clientAddress, char* pPacketData);
	bool CreateNewPlayer(const sockaddr_in& clientAddress);
    void UpdatePlayer(PlayerInputPacket* pPlayerInputPacket);
	bool ShutDown();

	bool UpdateWorldState(float deltaTime);

private:
	SOCKET m_oServerSocket = INVALID_SOCKET;
	std::string m_strServerIP;
	bool m_bIsServerRuning = false;
	std::vector<Player> m_oPlayers;

    float m_deltaTime = 0;

	int m_NextPlayerId = 1;
};