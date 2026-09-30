#include <iostream>
#include "Game.h"

int main(int argc, char* argv)
{
	std::unique_ptr<Game> game = std::make_unique<Game>();
	if (!game->Start())
		return -1;

	game->Run();

	return 0;
}