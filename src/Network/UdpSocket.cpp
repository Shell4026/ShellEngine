#include "UdpSocket.h"

#include "Core/Logger.h"

#include <asio.hpp>

namespace sh::network
{
	struct UdpSocket::Impl
	{
		asio::ip::udp::socket udpSocket;
		asio::ip::udp::endpoint receivedFrom;
	};
	UdpSocket::UdpSocket(const NetworkContext& ctx)
	{
		buffer.fill(0);

		asio::io_context& ioCtx = *reinterpret_cast<asio::io_context*>(ctx.GetNativeHandle());
		impl = std::make_unique<Impl>(UdpSocket::Impl{ asio::ip::udp::socket(ioCtx), asio::ip::udp::endpoint() });
	}
	UdpSocket::UdpSocket(UdpSocket&& other) noexcept :
		impl(std::move(other.impl)),
		buffer(other.buffer),
		receivedMessage(std::move(other.receivedMessage))
	{
	}
	UdpSocket::~UdpSocket()
	{
		if (impl != nullptr)
			impl->udpSocket.close();
	}
	SH_NET_API auto UdpSocket::operator=(UdpSocket&& other) noexcept -> UdpSocket&
	{
		if (this == &other)
			return *this;

		impl = std::move(other.impl);
		buffer = other.buffer;
		receivedMessage = std::move(other.receivedMessage);
		return *this;
	}
	SH_NET_API auto UdpSocket::Bind(uint16_t port) -> bool
	{
		impl->udpSocket.open(asio::ip::udp::v4());
		if (port != 0)
		{
			asio::error_code err;
			impl->udpSocket.bind(asio::ip::udp::endpoint{ asio::ip::udp::v4(), port }, err);
			if (err)
			{
				SH_ERROR_FORMAT("Error to bind: {}", err.message());
				return false;
			}
		}
		Receive();
		return true;
	}
	SH_NET_API void UdpSocket::Close()
	{
		impl->udpSocket.close();
	}
	SH_NET_API void UdpSocket::Send(const Packet& packet, const std::string& ip, uint16_t port)
	{
		if (!IsOpen())
		{
			SH_ERROR("Socket is not open!");
			return;
		}
		const asio::ip::udp::endpoint endPoint{ asio::ip::make_address(ip), port };

		auto buffer = std::make_unique<std::vector<uint8_t>>(core::Json::to_bson(packet.Serialize()));
		std::vector<uint8_t>* bufferRawPtr = buffer.get();
		impl->udpSocket.async_send_to(asio::buffer(*bufferRawPtr), endPoint,
			[buffer = std::move(buffer)](std::error_code ec, std::size_t sendBytes)
			{
				if (ec)
				{
					SH_INFO_FORMAT("Send failed: {} ({})", ec.message(), ec.value());
				}
			}
		);
	}
	SH_NET_API auto UdpSocket::GetReceivedMessage() -> std::optional<NetworkContext::Message>
	{
		if (!mu.try_lock())
			return {};
		std::lock_guard<std::mutex> lock{ mu, std::adopt_lock };
		if (receivedMessage.empty())
			return {};
		NetworkContext::Message msg = std::move(receivedMessage.front());
		receivedMessage.pop();
		return msg;
	}
	SH_NET_API auto UdpSocket::IsOpen() const -> bool
	{
		return impl->udpSocket.is_open();
	}
	void UdpSocket::Receive()
	{
		if (!IsOpen())
		{
			SH_ERROR("Socket is not open!");
			return;
		}

		impl->udpSocket.async_receive_from
		(
			asio::buffer(buffer), impl->receivedFrom,
			[this](std::error_code ec, std::size_t receivedBytes)
			{
				if (!ec && receivedBytes > 0)
				{
					const core::Json json = core::Json::from_bson(buffer.data(), receivedBytes, true, true);
					if (json.contains("id"))
					{
						static auto conatinerFactory = Packet::Factory::GetInstance();
						auto packet = conatinerFactory->Create(json["id"]);
						if (packet != nullptr)
						{
							packet->Deserialize(json);
							std::lock_guard<std::mutex> lock{ mu };

							NetworkContext::Message message{};
							message.senderIp = impl->receivedFrom.address().to_string();
							message.senderPort = static_cast<uint16_t>(impl->receivedFrom.port());
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
}//namespace