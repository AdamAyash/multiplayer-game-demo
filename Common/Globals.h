#pragma once

static class Globals
{
	Globals() = delete;
	~Globals() = delete;

public:
	static constexpr int nMaxPlayers = 16;
	static constexpr int nWindowWidth = 1280;
	static constexpr int nWindowHeight = 720;
};