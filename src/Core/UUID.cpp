#include "UUID.h"

#include <charconv>
#include <algorithm>
namespace sh::core
{
	SH_CORE_API UUID::UUID(std::string_view str)
	{
		// 32글자여야 함
		if (str.size() != uuid.size() * 8)
		{
			this->operator=(Generate());
			return;
		}
		for (int i = 0; i < str.size(); i += 8)
		{
			const std::string_view sub{ str.substr(i, 8) };
			uint32_t value{};
			auto [ptr, ec] = std::from_chars(sub.data(), sub.data() + sub.size(), value, 16);
			if (ec == std::errc{})
				uuid[i / 8] = value;
			else
			{
				this->operator=(GenerateEmptyUUID());
				return;
			}
		}
	}
	UUID::UUID(const std::array<uint32_t, 4>& uuid)
	{
		this->uuid = uuid;
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
	SH_CORE_API auto UUID::operator==(const UUID& other) const noexcept -> bool
	{
		for (int i = 0; i < uuid.size(); ++i)
		{
			if (uuid[i] != other.uuid[i])
				return false;
		}
		return true;
	}
	SH_CORE_API auto UUID::operator==(std::string_view str) const noexcept -> bool
	{
		if (str.size() != 32)
			return false;
		return operator==(core::UUID{ str });
	}
	SH_CORE_API auto UUID::operator!=(const UUID& other) const noexcept -> bool
	{
		return !operator==(other);
	}
	SH_CORE_API auto UUID::operator!=(std::string_view str) const noexcept -> bool
	{
		return !operator==(str);
	}
	SH_CORE_API auto UUID::Generate() -> UUID
	{
		UUID uuid{};
		for (int i = 0; i < 4; ++i)
			uuid.uuid[i] = Util::RandomRange(static_cast<uint32_t>(0), std::numeric_limits<uint32_t>::max());
		return uuid;
	}
	SH_CORE_API auto UUID::GenerateEmptyUUID() -> UUID
	{
		return UUID{ std::array<uint32_t, 4> { 0, 0, 0, 0 } };
	}
	SH_CORE_API auto UUID::ToString() const -> std::string
	{
		std::string uuidStr(32, '0');
		for (int i = 0; i < 4; ++i)
		{
			std::array<char, 9> buffer{ '0','0','0','0','0','0','0','0','\0' };
			auto [ptr, err] = std::to_chars(buffer.data(), buffer.data() + 8, uuid[i], 16);
			const std::size_t len = ptr - buffer.data();
			// ex) ab가 나왔다면, 000000ab로 저장해야한다.
			for (int j = 0; j < len; ++j)
				uuidStr[i * 8 + 8 - len + j] = buffer[j];
		}
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