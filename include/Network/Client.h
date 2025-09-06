#pragma once
#include "Export.h"

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
		struct Message
		{
			std::string message;
		};
	public:
		SH_NET_API void Connect(const std::string& ip, uint16_t port);
		SH_NET_API void Disconnect();
		SH_NET_API void Update();
		SH_NET_API void Send(const std::string& str);

		SH_NET_API auto GetReceivedMessage() -> std::optional<Message>;
	private:
		void Receive();
	private:
		asio::io_context ioContext;
		std::unique_ptr<asio::ip::udp::socket> socket;
		asio::ip::udp::endpoint serverEndpoint;

		std::array<char, 1024> buffer;

		std::queue<Message> receivedMessage;

		std::mutex mu;
	};
}//namespace