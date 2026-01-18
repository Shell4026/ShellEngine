#include "Component/UdpClient.h"
#include "Input.h"

#include <Network/StringPacket.h>

#include "Core/ThreadPool.h"

namespace sh::game
{
	SH_GAME_API UdpClient::UdpClient(GameObject& owner) :
		NetworkComponent(owner),
		socket(ctx)
	{
	}
	SH_GAME_API void UdpClient::OnDestroy()
	{
		socket.Close();
		if (networkThread.joinable())
			networkThread.join();

		Super::OnDestroy();
	}
	SH_GAME_API void UdpClient::Start()
	{
		socket.Bind();
		networkThread = std::thread(
			[this]()
			{
				while (socket.IsOpen())
				{
					ctx.Update();
				}
			}
		);
	}
	SH_GAME_API void UdpClient::Update()
	{
		if (Input::GetKeyPressed(Input::KeyCode::Space))
		{
			network::StringPacket packet{};
			packet.SetString("hello?");
			socket.Send(packet, serverIp, serverPort);
		}

		if (socket.IsOpen())
		{
			auto opt = socket.GetReceivedMessage();
			while (opt.has_value())
			{
				auto& message = opt.value();
				if (message.packet->GetId() == 0)
					SH_INFO_FORMAT("Received packet (id:0, {}) from server!", static_cast<network::StringPacket*>(message.packet.get())->GetString());
				opt = socket.GetReceivedMessage();
			}
		}
	}
	SH_GAME_API void UdpClient::SendPacket(const network::Packet& packet)
	{
		socket.Send(packet, serverIp, serverPort);
	}
}//namespace