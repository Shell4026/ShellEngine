#pragma once
#include "Export.h"
#include "Packet.h"

#include <string>
namespace sh::network
{
	class StringPacket : public Packet
	{
		SPACKET(StringPacket, 0)
	public:
		SH_NET_API auto GetId() const -> uint32_t override;
		SH_NET_API auto Serialize() const -> core::Json override;
		SH_NET_API void Deserialize(const core::Json& json) override;

		SH_NET_API void SetString(const std::string& str);
		SH_NET_API void SetString(std::string&& str);
		SH_NET_API auto GetString() const -> const std::string&;
	private:
		std::string str;
	};
}//namespace