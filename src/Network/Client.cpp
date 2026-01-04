#include "Client.h"
#include "Packet.h"

#include <asio.hpp>

#include "Core/Logger.h"
namespace sh::network
{
	struct Client::Impl
	{
		asio::io_context ioContext;
		std::unique_ptr<asio::ip::udp::socket> socket;
		asio::ip::udp::endpoint serverEndpoint;
	};

	Client::Client()
	{
		impl = std::make_unique<Impl>();
		receivedBuffer.resize(Packet::MAX_PACKET_SIZE, 0);
	}
	Client::~Client()
	{
		Disconnect();
	}
	SH_NET_API void Client::Connect(const std::string& ip, uint16_t port)
	{
		impl->serverEndpoint = asio::ip::udp::endpoint{ asio::ip::make_address(ip), port };

		impl->socket = std::make_unique<asio::ip::udp::socket>(impl->ioContext);
		impl->socket->open(asio::ip::udp::v4());

		Receive();
	}
	SH_NET_API void Client::Disconnect()
	{
		impl->ioContext.stop();
		if (impl->socket != nullptr)
		{
			impl->socket->close();
			impl->socket.reset();
		}
	}
	SH_NET_API void Client::Update()
	{
		impl->ioContext.run();
	}
	SH_NET_API void Client::Send(const Packet& packet)
	{
		if (impl->socket != nullptr)
		{
			auto buffer = std::make_unique<Buffer>(core::Json::to_bson(packet.Serialize()));
			Buffer* bufferRawPtr = buffer.get();
			impl->socket->async_send_to(asio::buffer(*bufferRawPtr), impl->serverEndpoint,
				[buffer = std::move(buffer)](std::error_code ec, std::size_t sendBytes)
				{
					if (ec)
						SH_INFO_FORMAT("Send failed: {} ({})", ec.message(), ec.value());
				}
			);
			//impl->socket->send_to(asio::buffer(core::Json::to_bson(packet.Serialize())), impl->serverEndpoint); // async_send_to로 나중에 비동기 생각
		}
	}
	SH_NET_API auto Client::GetReceivedPacket() -> std::unique_ptr<Packet>
	{
		std::lock_guard<std::mutex> lock{ mu };
		if (receivedPacket.empty())
			return {};
		std::unique_ptr<Packet> packet = std::move(receivedPacket.front());
		receivedPacket.pop();
		return packet;
	}
	void Client::Receive()
	{
		impl->socket->async_receive_from
		(
			asio::buffer(receivedBuffer), impl->serverEndpoint,
			[this](std::error_code ec, std::size_t receivedBytes)
			{
				if (!ec && receivedBytes > 0)
				{
					core::Json json = core::Json::from_bson(receivedBuffer.data(), receivedBytes, true, true);
					if (json.contains("id"))
					{
						static auto conatinerFactory = Packet::Factory::GetInstance();
						auto packet = conatinerFactory->Create(json["id"]);
						if (packet != nullptr)
						{
							packet->Deserialize(json);
							std::lock_guard<std::mutex> lock{ mu };
							receivedPacket.push(std::move(packet));
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
}//namespace