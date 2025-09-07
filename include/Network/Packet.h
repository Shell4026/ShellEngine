#pragma once
#include "Export.h"

#include "Core/ISerializable.h"
#include "Core/Factory.hpp"

#define SPACKET(PacketClass, id)\
struct PacketRegister##PacketClass\
{\
	PacketRegister##PacketClass()\
	{\
		auto factory = Packet::Factory::GetInstance();\
		if (!factory->HasKey(id))\
		{\
			factory->Register(id, \
				[]()->std::unique_ptr<Packet>\
				{\
					auto packet = std::make_unique<PacketClass>(); \
					return packet; \
				}\
			); \
		}\
	}\
	static auto Get() -> PacketRegister##PacketClass*\
	{\
		static PacketRegister##PacketClass registry{};\
		return &registry;\
	}\
};\
static inline PacketRegister##PacketClass* _packetRegister = PacketRegister##PacketClass::Get();

namespace sh::network
{
	class Packet : public core::ISerializable
	{
	public:
		using Factory = core::Factory<Packet, std::unique_ptr<Packet>, int32_t>;
		static constexpr uint32_t MAX_PACKET_SIZE = 1024;
	public:
		virtual auto GetId() const -> uint32_t = 0;

		SH_NET_API auto Serialize() const->core::Json override;
	};
}//namespace