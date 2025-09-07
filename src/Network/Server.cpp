#include "Server.h"

#include "Core/Logger.h"

namespace sh::network
{
	Server::Server()
	{
		std::memset(buffer.data(), 0, buffer.size());
	}
	SH_NET_API void Server::Start()
	{
		try
		{
			socket = std::make_unique<asio::ip::udp::socket>(ioContext, asio::ip::udp::endpoint{ asio::ip::udp::v4(), port });
			SH_INFO_FORMAT("Server starting on port {}", port);
			Receive();
		}
		catch (std::exception& e)
		{
			SH_ERROR_FORMAT("Server error: {}", e.what());
		}
	}
	SH_NET_API void Server::Stop()
	{
		ioContext.stop();
		if (socket != nullptr)
		{
			socket->close();
			socket.reset();
		}
	}
	SH_NET_API void Server::Run()
	{
		assert(socket != nullptr);
		ioContext.run();
	}
	SH_NET_API void Server::SetPort(uint16_t port)
	{
		this->port = port;
	}
	SH_NET_API auto Server::GetPort() const -> uint16_t
	{
		return port;
	}
	SH_NET_API auto Server::GetReceivedMessage() -> std::optional<Message>
	{
		std::lock_guard<std::mutex> lock{ mu };
		if (receivedMessage.empty())
			return {};
		Message msg = std::move(receivedMessage.front());
		receivedMessage.pop();
		return msg;
	}
	SH_NET_API void Server::Send(const Packet& packet, const asio::ip::udp::endpoint& to)
	{
		if (socket != nullptr)
		{
			socket->send_to(asio::buffer(core::Json::to_bson(packet.Serialize())), to); // async_send_to로 나중에 비동기 생각
		}
	}
	SH_NET_API auto Server::IsOpen() const -> bool
	{
		if (socket == nullptr)
			return false;
		return true;
	}
	void Server::Receive()
	{
		if (socket != nullptr)
		{
			socket->async_receive_from
			(
				asio::buffer(buffer), remoteEndpoint,
				[this](std::error_code ec, std::size_t receivedBytes)
				{
					if (!ec && receivedBytes > 0)
					{
						core::Json json = core::Json::from_bson(buffer.data(), receivedBytes, true, true);
						if (json.contains("id"))
						{
							static auto conatinerFactory = Packet::Factory::GetInstance();
							auto packet = conatinerFactory->Create(json["id"]);
							if (packet != nullptr)
							{
								packet->Deserialize(json);
								std::lock_guard<std::mutex> lock{ mu };
								receivedMessage.push({ remoteEndpoint, std::move(packet) });
							}
							else
								SH_ERROR_FORMAT("An unregistered packet has been received!");
						}
						else
							SH_ERROR("Error packet has been received! (No ID.)");
					}
					Receive();
				}
			);
		}
	}
}//namespace