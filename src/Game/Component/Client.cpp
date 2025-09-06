#include "Component/Client.h"
#include "Input.h"

#include "Core/ThreadPool.h"

namespace sh::game
{
	SH_GAME_API Client::Client(GameObject& owner) :
		NetworkComponent(owner)
	{
	}
	SH_GAME_API void Client::OnDestroy()
	{
		client.Disconnect();
		if (runFuture.valid())
			runFuture.get();
		Super::OnDestroy();
	}
	SH_GAME_API void Client::Start()
	{
		client.Connect(serverIp, serverPort);
	}
	SH_GAME_API void Client::BeginUpdate()
	{
		if (!runFuture.valid())
		{
			runFuture = core::ThreadPool::GetInstance()->AddTask(
				[&]()
				{
					client.Update();
				}
			);
		}
		else if (runFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
		{
			runFuture = core::ThreadPool::GetInstance()->AddTask(
				[&]()
				{
					client.Update();
				}
			);
		}
	}
	SH_GAME_API void Client::Update()
	{
		if (Input::GetKeyPressed(Input::KeyCode::Space))
			client.Send("hello?");

		auto opt = client.GetReceivedMessage();
		while (opt.has_value())
		{
			auto& message = opt.value();
			SH_INFO_FORMAT("Received message ({}) from server!", message.message);
			opt = client.GetReceivedMessage();
		}
	}
}//namespace