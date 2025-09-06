#pragma once
#include "Export.h"
#include "NetworkComponent.h"

#include "Network/Client.h"

#include <string>
#include <future>
namespace sh::game
{
	/// @brief 에코 클라이언트 컴포넌트
	class Client : public NetworkComponent
	{
		COMPONENT(Client, "network")
	public:
		SH_GAME_API Client(GameObject& owner);

		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void Update() override;
	private:
		network::Client client;

		PROPERTY(serverIp)
		std::string serverIp = "127.0.0.1";
		PROPERTY(serverPort)
		int serverPort = 40426;

		std::future<void> runFuture;
	};
}//namespace