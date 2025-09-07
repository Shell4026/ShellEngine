#pragma once
#include "Export.h"
#include "Packet.h"

#include <asio.hpp>

#include <memory>
#include <array>
#include <queue>
#include <mutex>
#include <optional>
namespace sh::network
{
	class Client
	{
	public:
		SH_NET_API void Connect(const std::string& ip, uint16_t port);
		SH_NET_API void Disconnect();
		SH_NET_API void Update();
		SH_NET_API void Send(const Packet& packet);

		SH_NET_API auto GetReceivedPacket() -> std::unique_ptr<Packet>;
	private:
		void Receive();
	private:
		asio::io_context ioContext;
		std::unique_ptr<asio::ip::udp::socket> socket;
		asio::ip::udp::endpoint serverEndpoint;

		std::array<uint8_t, Packet::MAX_PACKET_SIZE> buffer;

		std::queue<std::unique_ptr<Packet>> receivedPacket;

		std::mutex mu;
	};
}//namespace