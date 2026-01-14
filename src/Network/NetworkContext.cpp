#include "NetworkContext.h"

#include <asio.hpp>

namespace sh::network
{
	struct NetworkContext::Impl
	{
		asio::io_context ctx;
	};

	NetworkContext::NetworkContext()
	{
		impl = std::make_unique<Impl>();
	}

	NetworkContext::~NetworkContext()
	{
		Stop();
	}

	void NetworkContext::Update()
	{
		impl->ctx.run();
	}
	SH_NET_API void NetworkContext::Stop()
	{
		impl->ctx.stop();
	}
	SH_NET_API auto NetworkContext::GetNativeHandle() const -> void*
	{
		return &impl->ctx;
	}
}//namespace