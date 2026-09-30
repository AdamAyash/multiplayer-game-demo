#include "Server.h"
#include <iostream>

int main() 
{
	std::unique_ptr<Server> server = std::make_unique<Server>();

	if (!server->Start())
	{
		std::cerr << "Failed to start the server";
		return -1;
	}

	return 0;
}