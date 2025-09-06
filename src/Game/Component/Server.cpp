#include "Component/Server.h"

#include "Core/ThreadPool.h"
namespace sh::game
{
	Server::Server(GameObject& owner) :
		NetworkComponent(owner)
	{

	}
	SH_GAME_API void Server::OnDestroy()
	{
		server.Stop();
		if (runFuture.valid())
			runFuture.get();
		Super::OnDestroy();
	}
	SH_GAME_API void Server::Start()
	{
		server.Start();
	}
	SH_GAME_API void Server::BeginUpdate()
	{
		if (server.IsOpen())
		{
			if (!runFuture.valid())
			{
				runFuture = core::ThreadPool::GetInstance()->AddTask(
					[&]()
					{
						server.Run();
					}
				);
			}
			else if (runFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
			{
				runFuture = core::ThreadPool::GetInstance()->AddTask(
					[&]()
					{
						server.Run();
					}
				);
			}
		}
	}
	SH_GAME_API void Server::Update()
	{
		if (server.IsOpen())
		{
			auto opt = server.GetReceivedMessage();
			while (opt.has_value())
			{
				auto& message = opt.value();
				SH_INFO_FORMAT("Received message ({}) from {}", message.message, message.sender.address().to_string());

				server.Send(message.message, message.sender);

				opt = server.GetReceivedMessage();
			}
		}
	}
	SH_GAME_API void Server::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == core::Util::ConstexprHash("port"))
			server.SetPort(port);
	}
}//namespace