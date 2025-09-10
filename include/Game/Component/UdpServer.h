#pragma once
#include "../Export.h"
#include "NetworkComponent.h"

#include "Network/Server.h"
#include "Network/Packet.h"

#include <vector>
#include <future>
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
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void Update() override;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;
	protected:
		network::Server server;
	private:
		PROPERTY(port)
		int port = 4026;

		std::future<void> runFuture;
	};
}//namespace