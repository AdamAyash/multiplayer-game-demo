#include "pch.h"
#include "NetworkUtilities.h"
#include <ws2tcpip.h>


std::string NetworkUtilities::GetIPRepresentation(const addrinfo& addressInformation)
{
	char ipAddress[INET6_ADDRSTRLEN]{};

	switch (addressInformation.ai_family)
	{
	case AF_INET:
	{
		const auto* ipv4Address =
			reinterpret_cast<const sockaddr_in*>(
				addressInformation.ai_addr
				);

		if (inet_ntop(
			AF_INET,
			&ipv4Address->sin_addr,
			ipAddress,
			sizeof(ipAddress)
		) == nullptr)
		{
			return {};
		}

		return ipAddress;
	}

	case AF_INET6:
	{
		const auto* ipv6Address =
			reinterpret_cast<const sockaddr_in6*>(
				addressInformation.ai_addr
				);

		if (inet_ntop(
			AF_INET6,
			&ipv6Address->sin6_addr,
			ipAddress,
			sizeof(ipAddress)
		) == nullptr)
		{
			return {};
		}

		return ipAddress;
	}

	default:
		return {};
	}
}