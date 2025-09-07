#include "Packet.h"

namespace sh::network
{
	SH_NET_API auto Packet::Serialize() const -> core::Json
	{
		core::Json mainJson{};
		mainJson["id"] = GetId();
		return mainJson;
	}
}//namespace