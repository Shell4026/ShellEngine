#pragma once
#include "Core/IEvent.h"
#include "Packet.h"

namespace sh::network
{
	struct PacketEvent : core::IEvent
	{
	public:
		auto GetTypeHash() const -> std::size_t
		{
			return core::reflection::TypeTraits::GetTypeHash<PacketEvent>();
		};
		PacketEvent(const Packet* packet, const std::string& senderIp, uint16_t senderPort) :
			packet(packet), senderIp(senderIp), senderPort(senderPort)
		{}
		PacketEvent(const Packet* packet, std::string&& senderIp, uint16_t senderPort) :
			packet(packet), senderIp(std::move(senderIp)), senderPort(senderPort)
		{}
	public:
		const Packet* const packet;
		const std::string senderIp;
		const uint16_t senderPort;
	};
}//namespace