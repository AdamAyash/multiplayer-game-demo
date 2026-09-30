#pragma once

#include "Client.h"
#include <glad/glad.h>
#include "GLFW/glfw3.h"

class Game final
{
public:
	Game();
	~Game();

public:
	bool Start();
	bool ConnectToServer();
	void Run();
private:
	bool HandlePacket(PacketTypes ePacketType, char* pPacketData);
	void Update();
	void InitPlayer(const Packet& oPacket);
	void UpdateWordlState(const Packet& oPacket);

private:
	Client m_Client;
	int m_PlayerId = -1;
	GLFWwindow* m_pWindow = nullptr;
};