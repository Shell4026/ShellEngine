#include "TcpListener.h"

#include "Core/Logger.h"
#include "Core/ThreadSyncManager.h"

#include <asio.hpp>
namespace sh::network
{
	struct TcpListener::Impl
	{
		asio::ip::tcp::acceptor acceptor;
	};
	TcpListener::TcpListener(const NetworkContext& ctx)
	{
		asio::io_context& ioCtx = *reinterpret_cast<asio::io_context*>(ctx.GetNativeHandle());

		asio::ip::tcp::acceptor acceptor{ ioCtx };
		impl = std::make_unique<Impl>(Impl{ std::move(acceptor) });
	}
	TcpListener::~TcpListener()
	{
		impl->acceptor.close();
	}
	SH_NET_API auto TcpListener::Listen(uint16_t port) -> bool
	{
		impl->acceptor.open(asio::ip::tcp::v4());
		asio::error_code err;
		impl->acceptor.bind(asio::ip::tcp::endpoint{ asio::ip::tcp::v4(), port }, err);
		if (err)
		{
			SH_ERROR_FORMAT("Error to bind: {}", err.message());
			return false;
		}
		impl->acceptor.listen(asio::socket_base::max_listen_connections, err);
		if (err)
		{
			SH_ERROR_FORMAT("Error to listen: {}", err.message());
			return false;
		}

		Accept();
		return true;
	}
	SH_NET_API auto TcpListener::GetJoinedSocket() -> std::optional<TcpSocket>
	{
		if (!mu.try_lock())
			return {};

		std::lock_guard<std::mutex> lock{ mu, std::adopt_lock };
		if (joinedSocket.empty())
			return {};
		TcpSocket sock{ std::move(joinedSocket.front()) };
		joinedSocket.pop();

		return sock;
	}
	void TcpListener::Accept()
	{
		impl->acceptor.async_accept(
			[this](asio::error_code ec, asio::ip::tcp::socket socket)
			{
				if (ec)
				{
					SH_ERROR_FORMAT("Failed to accept: {}", ec.message());
					if (ec == asio::error::basic_errors::operation_aborted)
						return;
				}
				else
				{
					std::lock_guard<std::mutex> lock{ mu };
					joinedSocket.push(TcpSocket{ (void*)(&socket) });
				}
				Accept(); // 계속 받기
			}
		);
	}
}//namespace