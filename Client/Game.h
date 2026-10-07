#pragma once

#include "Client.h"
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "array"

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
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void InputHandling();


private:
	Client m_Client;
	GLFWwindow* m_pWindow = nullptr;
	std::array<Player*, Globals::nMaxPlayers> m_oPlayers{};
	glm::mat4 m_oModelMatrix = glm::mat4(1.);
	glm::mat4 m_oViewMatrix = glm::mat4(1.);
	glm::mat4 m_oProjectionMatrix = glm::mat4(1.);

	Player* m_oPlayer = nullptr;

	static std::vector<PlayerInputPacket> m_oPlayerInputPacketsQueue;
};