#pragma once
#include "Export.h"
#include "Packet.h"

#include <memory>
#include <string>
namespace sh::network
{
	class NetworkContext
	{
	public:
		SH_NET_API NetworkContext();
		SH_NET_API ~NetworkContext();

		SH_NET_API void Update();
		SH_NET_API void Stop();

		SH_NET_API auto GetNativeHandle() const -> void*;
	public:
		struct Message
		{
			std::string senderIp;
			uint16_t senderPort;
			std::unique_ptr<Packet> packet;
		};
	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}//namespace