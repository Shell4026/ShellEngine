#pragma once
#include "Export.h"
#include "Packet.h"

#include <memory>
#include <array>
#include <vector>
#include <queue>
#include <mutex>
#include <optional>
#include <cstdint>
namespace sh::network
{
	class Client
	{
	public:
		SH_NET_API Client();
		SH_NET_API ~Client();

		SH_NET_API void Connect(const std::string& ip, uint16_t port);
		SH_NET_API void Disconnect();
		SH_NET_API void Update();
		SH_NET_API void Send(const Packet& packet);

		SH_NET_API auto GetReceivedPacket() -> std::unique_ptr<Packet>;
	private:
		void Receive();
	private:
		using Buffer = std::vector<uint8_t>;
		struct Impl;
		std::unique_ptr<Impl> impl;

		Buffer receivedBuffer;
		std::queue<std::unique_ptr<Packet>> receivedPacket;

		std::mutex mu;
	};
}//namespace