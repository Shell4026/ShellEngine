#pragma once
#include "Export.h"
#include "PlayerInfo.h"

#include <asio.hpp>

#include <array>
#include <memory>
#include <queue>
#include <mutex>
#include <optional>
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
			asio::ip::udp::endpoint sender;
			std::string message;
		};
	public:
		SH_NET_API Server();

		SH_NET_API void Start();
		SH_NET_API void Stop();
		SH_NET_API void Run();

		SH_NET_API void SetPort(uint16_t port);
		SH_NET_API auto GetPort() const -> uint16_t;

		SH_NET_API auto GetReceivedMessage() -> std::optional<Message>;

		SH_NET_API void Send(const std::string& message, const asio::ip::udp::endpoint& to);

		SH_NET_API auto IsOpen() const -> bool;
	private:
		void Receive();
	private:
		asio::io_context ioContext;
		std::unique_ptr<asio::ip::udp::socket> socket;
		asio::ip::udp::endpoint remoteEndpoint;

		std::array<char, 1024> buffer;

		uint16_t port = 4026;

		std::queue<Message> receivedMessage;

		std::mutex mu;
	};
}//namespace