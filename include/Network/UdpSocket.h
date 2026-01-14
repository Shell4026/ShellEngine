#pragma once
#include "Export.h"
#include "NetworkContext.h"
#include "Packet.h"

#include <memory>
#include <array>
#include <queue>
#include <mutex>
#include <optional>
namespace sh::network
{
	class UdpSocket
	{
	public:
		SH_NET_API UdpSocket(const NetworkContext& ctx);
		SH_NET_API UdpSocket(UdpSocket&& other) noexcept;
		SH_NET_API ~UdpSocket();

		SH_NET_API auto operator=(UdpSocket&& other) noexcept -> UdpSocket&;

		/// @brief UDP소켓을 여는 함수
		/// @param port 포트, 0이면 OS에서 지정해준다.
		/// @return 성공 여부
		SH_NET_API auto Bind(uint16_t port = 0) -> bool;
		SH_NET_API void Close();
		SH_NET_API void Send(const Packet& packet, const std::string& ip, uint16_t port);

		SH_NET_API auto GetReceivedMessage() -> std::optional<NetworkContext::Message>;

		SH_NET_API auto IsOpen() const -> bool;
	private:
		void Receive();
	private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		std::array<uint8_t, Packet::MAX_PACKET_SIZE> buffer;
		std::queue<NetworkContext::Message> receivedMessage;

		std::mutex mu;
	};
}//namespace