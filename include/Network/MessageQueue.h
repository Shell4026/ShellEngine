#pragma once
#include "Export.h"
#include "NetworkContext.h"

#include "Core/ISyncable.h"

#include <queue>
#include <mutex>
#include <atomic>
#include <cstdint>
namespace sh::network
{
	/// @brief Tcp소켓에서 받은 메시지를 보관하는 메시지 큐. 스레드 안전하다.
	class MessageQueue
	{
	public:
		SH_NET_API void Push(NetworkContext::Message&& msg);

		SH_NET_API auto Pop() -> std::optional<NetworkContext::Message>;

		SH_NET_API auto IsEmpty() const -> bool;
		SH_NET_API auto GetSize() const -> std::size_t;
	private:
		std::queue<NetworkContext::Message> msgQueue;
		mutable std::mutex mu;
	};
}//namespace