#include "Component/UdpClient.h"
#include "Input.h"

#include <Network/StringPacket.h>

#include "Core/ThreadPool.h"

namespace sh::game
{
	SH_GAME_API UdpClient::UdpClient(GameObject& owner) :
		NetworkComponent(owner)
	{
	}
	SH_GAME_API void UdpClient::OnDestroy()
	{
		client.Disconnect();
		if (runFuture.valid())
			runFuture.get();

		Super::OnDestroy();
	}
	SH_GAME_API void UdpClient::Start()
	{
		client.Connect(serverIp, serverPort);
	}
	SH_GAME_API void UdpClient::BeginUpdate()
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
	SH_GAME_API void UdpClient::Update()
	{
		if (Input::GetKeyPressed(Input::KeyCode::Space))
		{
			network::StringPacket packet{};
			packet.SetString("hello?");
			client.Send(packet);
		}

		auto packet = client.GetReceivedPacket();
		while (packet != nullptr)
		{
			if (packet->GetId() == 0)
			{
				SH_INFO_FORMAT("Received packet (id:0, {}) from server!", static_cast<network::StringPacket*>(packet.get())->GetString());
			}
			
			packet = client.GetReceivedPacket();
		}
	}
	SH_GAME_API void UdpClient::SendPacket(const network::Packet& packet)
	{
		client.Send(packet);
	}
}//namespace