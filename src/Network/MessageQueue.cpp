#include "MessageQueue.h"
namespace sh::network
{
	SH_NET_API void MessageQueue::Push(NetworkContext::Message&& msg)
	{
		std::lock_guard<std::mutex> lock{ mu };
		msgQueue.push(std::move(msg));
	}
	SH_NET_API auto MessageQueue::Pop() -> std::optional<NetworkContext::Message>
	{
		if (!mu.try_lock())
			return {};

		std::lock_guard<std::mutex> lock{ mu, std::adopt_lock };
		if (msgQueue.empty())
			return {};

		NetworkContext::Message msg{ std::move(msgQueue.front()) };
		msgQueue.pop();

		return msg;
	}
	SH_NET_API auto MessageQueue::IsEmpty() const -> bool
	{
		std::lock_guard<std::mutex> lock{ mu };
		return msgQueue.empty();
	}
	SH_NET_API auto MessageQueue::GetSize() const -> std::size_t
	{
		std::lock_guard<std::mutex> lock{ mu };
		return msgQueue.size();
	}
}//namespace