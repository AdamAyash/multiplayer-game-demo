#pragma once

#include <Common.h>
#include <WinSock2.h>

class  Player final
{
public:
	Player();
	~Player();

public:
	int nId = -1;
	float x;
	float y;
	sockaddr_in oPlayerAddress;
};

struct PlayerState
{
	PlayerState()
	{
		SECURE_ZERO_MEMORY();
	}

	PlayerState operator = (const Player& player)
	{
		nId = player.nId;
		x = player.x;
		y = player.y;

		return *this;
	}

	int nId = -1;
	float x;
	float y;
};