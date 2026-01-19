#pragma once
#include "Export.h"
#include "NetworkContext.h"
#include "Packet.h"
#include "MessageQueue.h"

#include "Core/EventBus.h"

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
		SH_NET_API void SendBlocking(const Packet& packet);
		SH_NET_API void Close();
		SH_NET_API void ReadStart();

		/// @brief 전달 받은 메시지를 해당 큐로 집어넣게 설정한다.
		SH_NET_API void SetReceiveQueue(const std::shared_ptr<MessageQueue>& msgQueue);
		SH_NET_API auto GetReceiveQueue() const -> MessageQueue* { return receivedQueue.get(); }

		SH_NET_API auto GetIp() const -> const std::string& { return ip; }
		SH_NET_API auto GetPort() const-> uint16_t { return port; }
		SH_NET_API auto IsOpen() const -> bool;
	private:
		explicit TcpSocket(void* nativeSocketPtr);

		void WriteNext();
		void ReadHeader();
		void ReadBody();
	public:
		core::EventBus bus;
	private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		std::string ip;
		uint16_t port = 0;

		std::array<uint8_t, 4> header{};
		std::vector<uint8_t> body;
		std::deque<std::vector<uint8_t>> sendQueue;

		std::shared_ptr<MessageQueue> receivedQueue;

		std::mutex mu;
	};
}//namespace