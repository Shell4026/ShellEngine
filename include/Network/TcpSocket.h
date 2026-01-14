#pragma once
#include "Export.h"
#include "NetworkContext.h"
#include "Packet.h"

#include <memory>
#include <array>
#include <vector>
#include <deque>
#include <mutex>
namespace sh::network
{
	class TcpSocket
	{
		friend class TcpListener;
	public:
		SH_NET_API TcpSocket(const NetworkContext& ctx);
		SH_NET_API TcpSocket(TcpSocket&& other) noexcept;
		SH_NET_API ~TcpSocket();

		SH_NET_API auto operator=(TcpSocket&& other) noexcept -> TcpSocket&;

		SH_NET_API void Connect(const std::string& ip, uint16_t port);
		SH_NET_API void Send(const Packet& packet);
		SH_NET_API void Close();

		SH_NET_API auto GetReceivedMessage() -> std::optional<NetworkContext::Message>;

		SH_NET_API auto IsOpen() const -> bool;
	private:
		explicit TcpSocket(void* nativeSocketPtr);

		void WriteNext();
		void ReadHeader();
		void ReadBody();
	private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		std::array<uint8_t, 4> header{};
		std::vector<uint8_t> body;
		std::deque<std::vector<uint8_t>> sendQueue;
		std::queue<NetworkContext::Message> receivedMessage;

		std::mutex mu;
	};
}//namespace