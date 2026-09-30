#pragma once

#include <string>
#include <WinSock2.h>

static class NetworkUtilities
{
	NetworkUtilities() = delete;
	~NetworkUtilities() = delete;

public:
	static std::string GetIPRepresentation(const addrinfo& addressInfo);
};