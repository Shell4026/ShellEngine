#include "UUID.h"
#include "Util.h"

#include <charconv>
#include <mutex>
#include <algorithm>
namespace sh::core
{
	SH_CORE_API UUID::UUID(std::string_view str)
	{
		// 32글자
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
			uuidStr = str;
		}
		else
		{
			this->operator=(Generate());
		}
	}
	UUID::UUID(const std::array<uint32_t, 4>& uuid)
	{
		this->uuid = uuid;
		uuidStr.resize(32, '0');
		for (int i = 0; i < 4; ++i)
		{
			std::array<char, 9> buffer{ '0','0','0','0','0','0','0','0','\0' };
			auto [ptr, err] = std::to_chars(buffer.data(), buffer.data() + 8, uuid[i], 16);
			std::size_t len = ptr - buffer.data();
			// ex) ab가 나왔다면, 000000ab로 저장해야한다.
			for (int j = 0; j < len; ++j)
				uuidStr[i * 8 + 8 - len + j] = buffer[j];
		}
	}
	SH_CORE_API UUID::UUID(const UUID& other) noexcept
	{
		for (int i = 0; i < uuid.size(); ++i)
			uuid[i] = other.uuid[i];
		uuidStr = other.uuidStr;
	}
	SH_CORE_API UUID::UUID(UUID&& other) noexcept
	{
		for (int i = 0; i < uuid.size(); ++i)
			uuid[i] = other.uuid[i];
		uuidStr = std::move(other.uuidStr);
	}

	SH_CORE_API auto UUID::operator=(const UUID& other) noexcept -> UUID&
	{
		for (int i = 0; i < uuid.size(); ++i)
			uuid[i] = other.uuid[i];
		uuidStr = other.uuidStr;
		return *this;
	}
	SH_CORE_API auto UUID::operator=(UUID&& other) noexcept -> UUID&
	{
		for (int i = 0; i < uuid.size(); ++i)
			uuid[i] = other.uuid[i];
		uuidStr = std::move(other.uuidStr);
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
		static std::mutex mu{};

		std::lock_guard<std::mutex> lock{ mu };
		if (uuids.empty())
		{
			for (int i = 0; i < CHACHE_SIZE; ++i)
			{
				UUID uuid{};
				uuid.uuidStr.resize(32, '0');
				for (int i = 0; i < 4; ++i)
				{
					uuid.uuid[i] = Util::RandomRange(static_cast<uint32_t>(0), std::numeric_limits<uint32_t>::max());

					std::array<char, 9> buffer{ '0','0','0','0','0','0','0','0','\0' };
					auto [ptr, err] = std::to_chars(buffer.data(), buffer.data() + 8, uuid.uuid[i], 16);
					std::size_t len = ptr - buffer.data();
					// ex) ab가 나왔다면, 000000ab로 저장해야한다.
					for (int j = 0; j < len; ++j)
						uuid.uuidStr[i * 8 + 8 - len + j] = buffer[j];
				}
				uuids.push(std::move(uuid));
			}
		}
		UUID uuid = uuids.top();
		uuids.pop();
		return uuid;
	}
	SH_CORE_API auto UUID::GenerateEmptyUUID() -> UUID
	{
		return UUID{ "00000000000000000000000000000000" };
	}
	SH_CORE_API auto UUID::ToString() const -> const std::string&
	{
		return uuidStr;
	}
	SH_CORE_API auto UUID::GetRawData() const -> const std::array<uint32_t, 4>&
	{
		return uuid;
	}
	SH_CORE_API auto UUID::IsEmpty() const -> bool
	{
		for (uint32_t i : uuid)
		{
			if (i != 0)
				return false;
		}
		return true;
	}
}//namespace