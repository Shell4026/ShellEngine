#include "Server.h"

#include "Core/Logger.h"

#include <asio.hpp>

namespace sh::network
{
	struct Server::Impl
	{
		asio::io_context ioContext;
		std::unique_ptr<asio::ip::udp::socket> socket;
		asio::ip::udp::endpoint remoteEndpoint;
	};

	Server::Server()
	{
		impl = std::make_unique<Impl>();

		std::memset(buffer.data(), 0, buffer.size());
	}
	Server::~Server()
	{
		Stop();
	}
	SH_NET_API void Server::Start()
	{
		try
		{
			impl->socket = std::make_unique<asio::ip::udp::socket>(impl->ioContext, asio::ip::udp::endpoint{ asio::ip::udp::v4(), port });
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
		impl->ioContext.stop();
		if (impl->socket != nullptr)
		{
			impl->socket->close();
			impl->socket.reset();
		}
	}
	SH_NET_API void Server::Run()
	{
		assert(socket != nullptr);
		impl->ioContext.run();
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
	SH_NET_API void Server::Send(const Packet& packet, const std::string& ip, uint16_t port)
	{
		if (impl->socket != nullptr)
		{
			const asio::ip::udp::endpoint endPoint{ asio::ip::make_address(ip), port };

			auto buffer = std::make_unique<std::vector<uint8_t>>(core::Json::to_bson(packet.Serialize()));
			std::vector<uint8_t>* bufferRawPtr = buffer.get();
			impl->socket->async_send_to(asio::buffer(*bufferRawPtr), endPoint,
				[buffer = std::move(buffer)](std::error_code ec, std::size_t sendBytes)
				{
					if (ec)
					{
						SH_INFO_FORMAT("Send failed: {} ({})", ec.message(), ec.value());
					}
				}
			);

			//impl->socket->send_to(asio::buffer(core::Json::to_bson(packet.Serialize())), endPoint); // async_send_to로 나중에 비동기 생각
		}
	}
	SH_NET_API auto Server::IsOpen() const -> bool
	{
		if (impl->socket == nullptr)
			return false;
		return true;
	}
	void Server::Receive()
	{
		if (impl->socket != nullptr)
		{
			impl->socket->async_receive_from
			(
				asio::buffer(buffer), impl->remoteEndpoint,
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

								Message message{};
								message.senderIp = impl->remoteEndpoint.address().to_string();
								message.senderPort = static_cast<uint16_t>(impl->remoteEndpoint.port());
								message.packet = std::move(packet);

								receivedMessage.push(std::move(message));
							}
							else
								SH_ERROR("An unregistered packet has been received!");
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