#include "Component/Network/UdpServer.h"

#include "Network/StringPacket.h"
namespace sh::game
{
	UdpServer::UdpServer(GameObject& owner) :
		NetworkComponent(owner),
		socket(ctx)
	{

	}
	SH_GAME_API void UdpServer::Send(const network::Packet& packet, const std::string& ip, uint16_t port)
	{
		socket.Send(packet, ip, port);
	}
	SH_GAME_API void UdpServer::OnDestroy()
	{
		socket.Close();
		if (networkThread.joinable())
			networkThread.join();
		Super::OnDestroy();
	}
	SH_GAME_API void UdpServer::Start()
	{
		socket.Bind(port);
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
	SH_GAME_API void UdpServer::Update()
	{
		if (socket.IsOpen())
		{
			auto opt = socket.GetReceivedMessage();
			while (opt.has_value())
			{
				auto& message = opt.value();
				if (message.packet->GetId() == 0)
				{
					std::string str = static_cast<network::StringPacket*>(message.packet.get())->GetString();
					SH_INFO_FORMAT("Received packet (id: 0, {}) from {}", str, message.senderIp);

					network::StringPacket packet{};
					packet.SetString(std::move(str));
					socket.Send(packet, message.senderIp, message.senderPort);
				}
				opt = socket.GetReceivedMessage();
			}
		}
	}
	SH_GAME_API void UdpServer::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == core::Util::ConstexprHash("port"))
		{
			if (socket.IsOpen())
			{
				socket.Close();
				socket.Bind(port);
			}
		}
	}
}//namespace