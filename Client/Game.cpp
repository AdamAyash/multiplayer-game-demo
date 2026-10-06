#include "Game.h"
#include <iostream>
#include <string>
#include <vector>
#include "Shader.cpp"
#include "glm/matrix.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Player.h"

glm::vec3 m_oPlayerPosition = glm::vec3(0);
float m_fPlayerVelocity = 2.0;

Game::Game() 
{
}

Game::~Game()
{
}

bool Game::Start()
{
	std::cout << "Connecting to server...." << std::endl;

	bool connected = false;

	do {
		connected = ConnectToServer();
	} while (!connected);

	if (!glfwInit())
	{
		std::cerr << "Failed to init glfw" << std::endl;
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
		return false;
	}
	
	glViewport(0, 0, Globals::nWindowWidth, Globals::nWindowHeight);
	glfwSetFramebufferSizeCallback(m_pWindow, Game::framebuffer_size_callback);
	glfwSetKeyCallback(m_pWindow, Game::key_callback);

	return true;
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

	char* pPacketData = nullptr;
	PacketTypes ePacketType = Undefined;
	m_Client.ReceivePackets(ePacketType, pPacketData);

	if (!HandlePacket(ePacketType, pPacketData))
	{
		return false;
	}

	std::cout << "Succsessfully connected to the server." << std::endl;

	return true;
}

void Game::Run()
{
	m_oPlayerPosition = glm::vec3((float)Globals::nWindowWidth / 2, (float)Globals::nWindowHeight / 2, 0.0);
	//m_oViewMatrix = glm::translate(m_oViewMatrix, glm::vec3(0, 0, 3.f));
	m_oProjectionMatrix = glm::ortho(0.0f, (float)Globals::nWindowWidth, 0.0f, (float)Globals::nWindowHeight, -0.1f, 1.0f);

	float vertices[] = {
	 0.5f,  0.5f, 0.0f,  // top right
	 0.5f, -0.5f, 0.0f,  // bottom right
	-0.5f, -0.5f, 0.0f,  // bottom 
	-0.5f,  0.5f, 0.0f   // top left 
	};

	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	unsigned int nVao = 0;
	glGenVertexArrays(1, &nVao);
	glBindVertexArray(nVao);

	unsigned int nEbo = 0;
	glGenBuffers(1, &nEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	unsigned int nVbo = 0;
	glGenBuffers(1, &nVbo);
	glBindBuffer(GL_ARRAY_BUFFER, nVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	Shader oShader("VertexShader.vert", "FragmentShader.frag");

	while (!glfwWindowShouldClose(m_pWindow))
	{
		Update();

		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(nVao);

		for (int i = 0; i < m_oPlayers.size(); ++i)
		{
			const Player* pCurrentPlayer = m_oPlayers[i];
			if (!pCurrentPlayer)
				continue;

			glm::vec3 oPosition = glm::vec3(pCurrentPlayer->x, pCurrentPlayer->y, 0);

			m_oModelMatrix = glm::mat4(1.);
			m_oModelMatrix = glm::translate(m_oModelMatrix, oPosition);
			m_oModelMatrix = glm::scale(m_oModelMatrix, glm::vec3(50, 50, 0));

			oShader.use();
			oShader.setMatrix4("modelMatrix", m_oModelMatrix);
			oShader.setMatrix4("viewMatrix", m_oViewMatrix);
			oShader.setMatrix4("projectionMatrix", m_oProjectionMatrix);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

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

	std::cout << "Successfully connected to the server." << std::endl;
	std::cout << "Player ID: " << m_PlayerId << std::endl;
}

void Game::UpdateWordlState(const Packet& oPacket)
{
	const WorldStatePacket* oWorldStatePacket = &reinterpret_cast<const WorldStatePacket&>(oPacket);
	if (!oWorldStatePacket)
		return;

	for (int i = 0; i < oWorldStatePacket->playerCount; ++i)
	{
		Player* pCurrentPlayer = m_oPlayers[i];
		if (pCurrentPlayer == nullptr)
		{
			pCurrentPlayer = new Player();
			pCurrentPlayer->nId = oWorldStatePacket->players[i].nId;
			pCurrentPlayer->x = oWorldStatePacket->players[i].x;
			pCurrentPlayer->y = oWorldStatePacket->players[i].y;

			m_oPlayers[i] = pCurrentPlayer;
		}
	}
}

void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void Game::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		m_oPlayerPosition.x -= m_fPlayerVelocity;
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		m_oPlayerPosition.x += m_fPlayerVelocity;
	}
}