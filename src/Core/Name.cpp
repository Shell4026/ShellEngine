#include "Name.h"
#include "Util.h"
namespace sh::core
{
	std::unordered_map<std::size_t, std::string> Name::map{};
	std::shared_mutex Name::mu{};

	Name::Name(std::string_view str) :
		hash(Util::ConstexprHash(str))
	{
		{
			std::shared_lock lock{ mu };
			auto it = map.find(hash);
			if (it != map.end())
				return;
		}
		// 새로운 문자열을 쓰는 작업은 정말 드물게 일어날 것으로 예상된다.
		{
			std::unique_lock uniqueLock{ mu };
			if (map.find(hash) == map.end());
				map.insert({ hash, std::string{str} });
		}
	}
	Name::Name(const Name& other) noexcept :
		hash(other.hash)
	{
	}
	Name::Name(Name&& other) noexcept :
		hash(other.hash)
	{
	}
	Name::~Name()
	{
	}

	SH_CORE_API auto Name::operator=(const Name& other) noexcept -> Name&
	{
		hash = other.hash;
		return *this;
	}
	SH_CORE_API auto Name::operator=(Name&& other) noexcept -> Name&
	{
		hash = other.hash;
		return *this;
	}
	SH_CORE_API auto Name::operator==(const Name& other) const -> bool
	{
		return hash == other.hash;
	}
	SH_CORE_API auto Name::operator!=(const Name& other) const -> bool
	{
		return hash != other.hash;
	}
	SH_CORE_API auto Name::operator==(const std::string_view str) const -> bool
	{
		return hash == core::Util::ConstexprHash(str);
	}
	SH_CORE_API auto Name::operator!=(const std::string_view str) const -> bool
	{
		return hash != core::Util::ConstexprHash(str);
	}

	SH_CORE_API Name::operator const std::string& () const
	{
		return ToString();
	}

	SH_CORE_API auto Name::ToString() const -> const std::string&
	{
		std::shared_lock lock{ mu };
		const std::string& result = map[hash];
		return result;
	}
}//namespace