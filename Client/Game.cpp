#include "Game.h"
#include <iostream>
#include <string>


Game::Game() 
{
}

Game::~Game()
{
}

bool Game::Start()
{
	if (!ConnectToServer())
	{
		//TODO ERROR;
		return false;
	}

	if (!glfwInit())
	{

		return false;
	}

	glfwWindowHint(GLFW_VERSION_MAJOR, 330);
	glfwWindowHint(GLFW_VERSION_MINOR, 330);
	glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, 1);

	m_pWindow = glfwCreateWindow(Globals::nWindowWidth, Globals::nWindowHeight, "Multiplayer Game", NULL, NULL);
	glfwMakeContextCurrent(m_pWindow);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, Globals::nWindowWidth, Globals::nWindowHeight);
}

bool Game::ConnectToServer()
{
	std::string strIPAddress = "127.0.0.1";
	std::string portNumber = "3000";

	if (!m_Client.ConnectToServer(strIPAddress, portNumber))
	{
		std::cerr << "Error connecting to server" << std::endl;
		return false;
	}

	return true;
}

void Game::Run()
{
	while (!glfwWindowShouldClose(m_pWindow))
	{

		Update();

		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(m_pWindow);
		glfwPollEvents();
	}

	glfwTerminate();
}

bool Game::HandlePacket(PacketTypes ePacketType, char* pPacketData)
{
	switch (ePacketType)
	{
	case Welcome:
	{
		WelcomePacket* pWelcolmePlayer = reinterpret_cast<WelcomePacket*>(pPacketData);
		InitPlayer(*pWelcolmePlayer);
		break;
	}
	case WorldState:
	{
		WorldStatePacket* pWordlState = reinterpret_cast<WorldStatePacket*>(pPacketData);
		UpdateWordlState(*pWordlState);
		break;
	}
	default:
		std::cerr << "Invalid packet type" << std::endl;
		return false;
	}
	return true;
}

void Game::Update() 
{
	char* pPacketData = nullptr;
	PacketTypes ePacketType = Undefined;
	if (m_Client.ReceivePackets(ePacketType, pPacketData))
	{
		if (!HandlePacket(ePacketType, pPacketData))
		{
			std::cerr << "Handle packet failed." << std::endl;
		}
	}

	delete[] pPacketData;
}

void Game::InitPlayer(const Packet& oPacket)
{
	const WelcomePacket* oWelcomePacket = &reinterpret_cast<const WelcomePacket&>(oPacket);
	if (!oWelcomePacket)
		return;

	m_PlayerId = oWelcomePacket->id;
	std::cout << "Player ID: " << m_PlayerId << std::endl;
}

void Game::UpdateWordlState(const Packet& oPacket)
{
	const WorldStatePacket* oWorldStatePacket = &reinterpret_cast<const WorldStatePacket&>(oPacket);
	if (!oWorldStatePacket)
		return;
}