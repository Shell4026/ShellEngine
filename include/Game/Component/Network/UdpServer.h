#pragma once
#include "Game/Export.h"
#include "NetworkComponent.h"

#include "Network/UdpSocket.h"
#include "Network/Packet.h"

#include <vector>
#include <thread>
namespace sh::game
{
	/// @brief UDP 에코 서버 컴포넌트
	class UdpServer : public NetworkComponent
	{
		COMPONENT(UdpServer, "network")
	public:
		SH_GAME_API UdpServer(GameObject& owner);

		SH_GAME_API void Send(const network::Packet& packet, const std::string& ip, uint16_t port);

		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void Update() override;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API auto GetPort() const -> int { return port; }
	protected:
		network::NetworkContext ctx;
		network::UdpSocket socket;
	private:
		PROPERTY(port)
		int port = 4026;

		std::thread networkThread;
	};
}//namespace