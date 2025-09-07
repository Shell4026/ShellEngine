#pragma once
#include "Export.h"
#include "Packet.h"

#include <array>
#include <memory>
#include <queue>
#include <mutex>
#include <optional>
#include <cstdint>
#ifdef SetPort
#undef SetPort;
#endif
namespace sh::network
{
	class Server
	{
	public:
		struct Message
		{
			std::string senderIp;
			uint16_t senderPort;
			std::unique_ptr<Packet> packet;
		};
	public:
		SH_NET_API Server();
		SH_NET_API ~Server();

		SH_NET_API void Start();
		SH_NET_API void Stop();
		SH_NET_API void Run();

		SH_NET_API void SetPort(uint16_t port);
		SH_NET_API auto GetPort() const -> uint16_t;

		SH_NET_API auto GetReceivedMessage() -> std::optional<Message>;

		SH_NET_API void Send(const Packet& packet, const std::string& ip, uint16_t port);

		SH_NET_API auto IsOpen() const -> bool;
	private:
		void Receive();
	private:
		struct Impl;

		std::unique_ptr<Impl> impl;

		uint16_t port = 4026;

		std::array<uint8_t, Packet::MAX_PACKET_SIZE> buffer;
		std::queue<Message> receivedMessage;

		std::mutex mu;
	};
}//namespace