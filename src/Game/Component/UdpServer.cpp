#include "Component/UdpServer.h"

#include "Network/StringPacket.h"
namespace sh::game
{
	UdpServer::UdpServer(GameObject& owner) :
		NetworkComponent(owner)
	{

	}
	SH_GAME_API void UdpServer::Send(const network::Packet& packet, const std::string& ip, uint16_t port)
	{
		server.Send(packet, ip, port);
	}
	SH_GAME_API void UdpServer::OnDestroy()
	{
		server.Stop();
		if (networkThread.joinable())
			networkThread.join();
		Super::OnDestroy();
	}
	SH_GAME_API void UdpServer::Start()
	{
		server.Start();
		networkThread = std::thread(
			[this]()
			{
				while (server.IsOpen())
				{
					server.Run();
				}
			}
		);
	}
	SH_GAME_API void UdpServer::Update()
	{
		if (server.IsOpen())
		{
			auto opt = server.GetReceivedMessage();
			while (opt.has_value())
			{
				auto& message = opt.value();
				if (message.packet->GetId() == 0)
				{
					std::string str = static_cast<network::StringPacket*>(message.packet.get())->GetString();
					SH_INFO_FORMAT("Received packet (id: 0, {}) from {}", str, message.senderIp);

					network::StringPacket packet{};
					packet.SetString(std::move(str));
					server.Send(packet, message.senderIp, message.senderPort);
				}
				opt = server.GetReceivedMessage();
			}
		}
	}
	SH_GAME_API void UdpServer::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == core::Util::ConstexprHash("port"))
			server.SetPort(port);
	}
}//namespace