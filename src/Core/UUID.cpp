#include "PCH.h"
#include "UUID.h"
#include "Util.h"

#include <charconv>

namespace sh::core
{
	SH_CORE_API UUID::UUID(std::string_view str)
	{
		if (str.size() == uuid.size() * 8)
		{
			try
			{
				for (int i = 0; i < str.size(); i += 8)
				{
					std::string sub{ str.substr(i, 8) };
					uuid[i / 8] = std::stoul(sub, nullptr, 16);
				}
					
			}
			catch (const std::exception& e)
			{
				this->operator=(Generate());
			}
		}
		else
		{
			this->operator=(Generate());
		}
	}
	SH_CORE_API UUID::UUID(const UUID& other) noexcept
	{
		for (int i = 0; i < uuid.size(); ++i)
			uuid[i] = other.uuid[i];
	}

	SH_CORE_API auto UUID::operator=(const UUID& other) noexcept -> UUID&
	{
		for (int i = 0; i < uuid.size(); ++i)
			uuid[i] = other.uuid[i];
		return *this;
	}

	SH_CORE_API bool UUID::operator==(const UUID& other) const noexcept
	{
		for (int i = 0; i < uuid.size(); ++i)
		{
			if (uuid[i] != other.uuid[i])
				return false;
		}
		return true;
	}
	SH_CORE_API bool UUID::operator!=(const UUID& other) const noexcept
	{
		for (int i = 0; i < uuid.size(); ++i)
		{
			if (uuid[i] != other.uuid[i])
				return true;
		}
		return false;
	}

	SH_CORE_API auto UUID::Generate() -> UUID
	{
		static std::stack<UUID> uuids;
		if (uuids.empty())
		{
			for (int i = 0; i < CHACHE_SIZE; ++i)
			{
				UUID uuid{};
				for (int i = 0; i < 4; ++i)
					uuid.uuid[i] = Util::RandomRange(static_cast<uint32_t>(0), std::numeric_limits<uint32_t>::max());
				uuids.push(std::move(uuid));
			}
		}
		UUID uuid = uuids.top();
		uuids.pop();
		return uuid;
	}

	SH_CORE_API auto UUID::ToString() const -> std::string
	{
		std::string result{};
		result.reserve(32);
		for (int i = 0; i < 4; ++i)
		{
			std::array<char, 9> buffer{ '0','0','0','0','0','0','0','0','\0' };
			std::to_chars(buffer.data(), buffer.data() + 8, uuid[i], 16);
			result += std::string{ buffer.data(), 8 };
		}
		return result;
	}
}//namespace