#pragma once
#include "../Export.h"
#include "NetworkComponent.h"

#include "Network/UdpSocket.h"

#include <string>
#include <thread>
namespace sh::game
{
	/// @brief 에코 클라이언트 컴포넌트
	class UdpClient : public NetworkComponent
	{
		COMPONENT(UdpClient, "network")
	public:
		SH_GAME_API UdpClient(GameObject& owner);

		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void Update() override;

		SH_GAME_API void SendPacket(const network::Packet& packet);

		SH_GAME_API auto GetServerIP() const -> const std::string& { return serverIp; }
		SH_GAME_API auto GetServerPort() const -> int { return serverPort; }
	protected:
		network::NetworkContext ctx;
		network::UdpSocket socket;
	private:
		PROPERTY(serverIp)
		std::string serverIp = "127.0.0.1";
		PROPERTY(serverPort)
		int serverPort = 4026;

		std::thread networkThread;
	};
}//namespace