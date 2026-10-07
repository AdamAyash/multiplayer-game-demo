#pragma once
#include <cstdint>
#include "Common.h"
#include <Player.h>
#include "Globals.h"

enum PacketTypes : uint8_t
{
	Undefined = -1,
	Connect = 1,
	HandShake,
	CreatePlayer,
	Welcome,
	WorldState,
	PlayerInput
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

struct HandShakePacket : public Packet
{
	HandShakePacket()
	{
		SECURE_ZERO_MEMORY();
		oPacketHeader.ePacketType = HandShake;
	}

	uint8_t isClientConnected;
};

struct CreatePlayerPacket : public Packet
{
	CreatePlayerPacket()
	{
		SECURE_ZERO_MEMORY();
		oPacketHeader.ePacketType = CreatePlayer;
	}

	uint8_t id;
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

struct PlayerInputPacket : public Packet
{
	PlayerInputPacket()
	{
		SECURE_ZERO_MEMORY();
		oPacketHeader.ePacketType = PlayerInput;
	}

	int id;
	float movX;
	float movY;
	float velocity;
};

#pragma pack(pop)