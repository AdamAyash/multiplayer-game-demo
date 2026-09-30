#pragma once
#include <cstdint>
#include "Common.h"
#include <Player.h>
#include "Globals.h"

enum PacketTypes : uint8_t
{
	Undefined = -1,
	Connect = 1,
	Welcome,
	WorldState
};

#pragma pack(push, 1)

struct PacketHeader
{
	PacketHeader()
	{
		SECURE_ZERO_MEMORY();
	}

	PacketTypes ePacketType;
};

struct Packet
{
	Packet()
	{
		SECURE_ZERO_MEMORY();
	}

public:
	PacketHeader oPacketHeader;
};

struct ConnectPacket : public Packet
{
	ConnectPacket()
	{
		SECURE_ZERO_MEMORY();
		oPacketHeader.ePacketType = Connect;
	}
};

struct WelcomePacket : public Packet
{
	WelcomePacket()
	{
		SECURE_ZERO_MEMORY();
		oPacketHeader.ePacketType = Welcome;
	}

	uint8_t id;
};

struct WorldStatePacket : public Packet
{
	WorldStatePacket() 
	{
		SECURE_ZERO_MEMORY();
		oPacketHeader.ePacketType = WorldState;
	}

	int playerCount;
	PlayerState players[Globals::nMaxPlayers];
};

#pragma pack(pop)