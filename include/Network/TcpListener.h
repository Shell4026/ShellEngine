#pragma once
#include "Export.h"
#include "NetworkContext.h"
#include "TcpSocket.h"

#include <memory>
#include <cstdint>
#include <queue>
#include <optional>
#include <mutex>
namespace sh::network
{
	/// @brief 비동기적으로 tcp연결을 받는 클래스. 스레드 안전하다.
	class TcpListener
	{
	public:
		SH_NET_API TcpListener(const NetworkContext& ctx);
		SH_NET_API ~TcpListener();

		SH_NET_API auto Listen(uint16_t port) -> bool;

		SH_NET_API auto GetJoinedSocket() -> std::optional<TcpSocket>;
	private:
		void Accept();
	private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		std::queue<TcpSocket> joinedSocket;

		std::mutex mu;
	};
}//namespace