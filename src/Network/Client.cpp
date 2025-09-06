#include "Client.h"

namespace sh::network
{
	SH_NET_API void Client::Connect(const std::string& ip, uint16_t port)
	{
		socket = std::make_unique<asio::ip::udp::socket>(ioContext);

		serverEndpoint = asio::ip::udp::endpoint{ asio::ip::make_address(ip), port };

		socket->open(asio::ip::udp::v4());
		Receive();
	}
	SH_NET_API void Client::Disconnect()
	{
		ioContext.stop();
		if (socket != nullptr)
		{
			socket->close();
			socket.reset();
		}
	}
	SH_NET_API void Client::Update()
	{
		ioContext.run();
	}
	SH_NET_API void Client::Send(const std::string& str)
	{
		if (socket != nullptr)
			socket->send_to(asio::buffer(str), serverEndpoint); // async_send_to로 나중에 비동기 생각
	}
	SH_NET_API auto Client::GetReceivedMessage() -> std::optional<Message>
	{
		std::lock_guard<std::mutex> lock{ mu };
		if (receivedMessage.empty())
			return {};
		Message msg = std::move(receivedMessage.front());
		receivedMessage.pop();
		return msg;
	}
	void Client::Receive()
	{
		socket->async_receive_from
		(
			asio::buffer(buffer), serverEndpoint,
			[this](std::error_code ec, std::size_t receivedBytes)
			{
				if (!ec && receivedBytes > 0)
				{
					std::string msg(buffer.data(), receivedBytes);
					std::lock_guard<std::mutex> lock{ mu };
					receivedMessage.push({ std::move(msg) });
				}
				Receive();
			}
		);
	}
}//namespace