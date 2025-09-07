#include "Client.h"
#include "Packet.h"

#include "Core/Logger.h"
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
	SH_NET_API void Client::Send(const Packet& packet)
	{
		if (socket != nullptr)
		{
			socket->send_to(asio::buffer(core::Json::to_bson(packet.Serialize())), serverEndpoint); // async_send_to로 나중에 비동기 생각
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
		socket->async_receive_from
		(
			asio::buffer(buffer), serverEndpoint,
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
							receivedPacket.push(std::move(packet));
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
}//namespace