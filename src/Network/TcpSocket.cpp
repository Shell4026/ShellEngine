#include "TcpSocket.h"

#include "Core/Logger.h"

#include <asio.hpp>

namespace sh::network
{
	struct TcpSocket::Impl
	{
		asio::ip::tcp::socket socket;
	};

	TcpSocket::TcpSocket(const NetworkContext& ctx)
	{
		asio::io_context& ioCtx = *reinterpret_cast<asio::io_context*>(ctx.GetNativeHandle());
		
		asio::ip::tcp::socket socket{ ioCtx };
		socket.open(asio::ip::tcp::v4());

		impl = std::make_unique<Impl>(Impl{ std::move(socket) });
	}
	TcpSocket::TcpSocket(TcpSocket&& other) noexcept :
		impl(std::move(other.impl)),
		header(other.header),
		body(std::move(other.body)),
		sendQueue(std::move(other.sendQueue)),
		receivedMessage(std::move(other.receivedMessage))
	{
	}
	TcpSocket::~TcpSocket()
	{
		impl->socket.close();
	}
	SH_NET_API auto TcpSocket::operator=(TcpSocket&& other) noexcept -> TcpSocket&
	{
		if (this == &other)
			return *this;

		impl = std::move(other.impl);
		header = other.header;
		body = std::move(other.body);
		sendQueue = std::move(other.sendQueue);
		receivedMessage = std::move(other.receivedMessage);

		return *this;
	}
	SH_NET_API void TcpSocket::Connect(const std::string& ip, uint16_t port)
	{
		asio::ip::tcp::endpoint endPoint{ asio::ip::make_address(ip), port };

		impl->socket.async_connect(endPoint,
			[this](asio::error_code ec)
			{
				if (ec)
					SH_ERROR_FORMAT("Failed to connect: {}", ec.message());
				else
					ReadHeader();
			}
		);
	}
	SH_NET_API void TcpSocket::Send(const Packet& packet)
	{
		const std::vector<uint8_t> data(core::Json::to_bson(packet.Serialize()));

		const uint32_t len = static_cast<uint32_t>(data.size());

		std::vector<uint8_t> sendData;
		// 헤더에 리틀엔디안으로 데이터 길이 기록
		sendData.resize(4 + data.size());
		sendData[0] = (len >> 0) & 0xFF;
		sendData[1] = (len >> 8) & 0xFF;
		sendData[2] = (len >> 16) & 0xFF;
		sendData[3] = (len >> 24) & 0xFF;
		// Body에 데이터 기록
		std::memcpy(sendData.data() + 4, sendData.data(), data.size());

		std::lock_guard<std::mutex> lock{ mu };
		bool bWasEmpty = sendQueue.empty();
		sendQueue.push_back(std::move(sendData));
		if (bWasEmpty)
			WriteNext();
	}
	SH_NET_API void TcpSocket::Close()
	{
		impl->socket.close();
	}
	SH_NET_API auto TcpSocket::GetReceivedMessage() -> std::optional<NetworkContext::Message>
	{
		std::lock_guard<std::mutex> lock{ mu };
		if (receivedMessage.empty())
			return {};
		NetworkContext::Message msg = std::move(receivedMessage.front());
		receivedMessage.pop();
		return msg;
	}
	SH_NET_API auto TcpSocket::IsOpen() const -> bool
	{
		return impl->socket.is_open();
	}
	TcpSocket::TcpSocket(void* nativeSocketPtr)
	{
		auto socketPtr = reinterpret_cast<asio::ip::tcp::socket*>(nativeSocketPtr);
		impl = std::make_unique<Impl>(Impl{ std::move(*socketPtr) });

		ReadHeader();
	}
	void TcpSocket::WriteNext()
	{
		asio::async_write(impl->socket, asio::buffer(sendQueue.front()),
			[this](std::error_code ec, std::size_t)
			{
				if (ec) 
				{
					SH_INFO_FORMAT("Send failed: {} ({})", ec.message(), ec.value());
					return;
				}
				std::lock_guard<std::mutex> lock{ mu };
				sendQueue.pop_front();
				if (!sendQueue.empty())
					WriteNext();
			}
		);
	}
	void TcpSocket::ReadHeader()
	{
		asio::async_read(impl->socket, asio::buffer(header),
			[this](std::error_code ec, std::size_t)
			{
				if (ec)
				{
					SH_INFO_FORMAT("Read hedaer failed: {} ({})", ec.message(), ec.value());
					return;
				}
				const uint32_t len =
					(uint32_t)header[0] |
					((uint32_t)header[1] << 8) |
					((uint32_t)header[2] << 16) |
					((uint32_t)header[3] << 24);

				constexpr uint32_t kMaxPacket = 1 * 1024 * 1024;
				if (len == 0 || len > kMaxPacket) 
				{ 
					Close(); 
					return; 
				}

				body.resize(len);
				ReadBody();
			}
		);
	}
	void TcpSocket::ReadBody()
	{
		asio::async_read(impl->socket, asio::buffer(body),
			[this](std::error_code ec, std::size_t)
			{
				if (ec)
				{
					SH_INFO_FORMAT("Read body failed: {} ({})", ec.message(), ec.value());
					return;
				}
				core::Json json = core::Json::from_bson(body.data(), body.size(), true, true);
				if (json.contains("id"))
				{
					static auto conatinerFactory = Packet::Factory::GetInstance();
					auto packet = conatinerFactory->Create(json["id"]);
					if (packet != nullptr)
					{
						packet->Deserialize(json);
						
						asio::error_code err;
						auto ep = impl->socket.remote_endpoint(err);
						if (err)
						{
							SH_ERROR_FORMAT("{}", err.message());
						}
						else
						{
							NetworkContext::Message message{};
							message.senderIp = ep.address().to_string();
							message.senderPort = static_cast<uint16_t>(ep.port());
							message.packet = std::move(packet);

							std::lock_guard<std::mutex> lock{ mu };
							receivedMessage.push(std::move(message));
						}
					}
					else
						SH_ERROR("An unregistered packet has been received!");
				}
				else
					SH_ERROR("Error packet has been received! (No ID.)");

				ReadHeader();
			}
		);
	}
}//namespace