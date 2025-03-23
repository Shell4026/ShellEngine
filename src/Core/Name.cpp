#include "Name.h"
#include "Util.h"

namespace sh::core
{
	std::unordered_map<std::size_t, std::string> Name::map{};
	SpinLock Name::lock{};

	Name::Name(std::string_view str) :
		hash(Util::ConstexprHash(str))
	{
		lock.Lock();
		auto it = map.find(hash);
		if (it == map.end())
			map.insert({ hash, std::string{str} });
		lock.UnLock();
	}
	Name::~Name()
	{
	}

	SH_CORE_API auto Name::operator==(const Name& other) const -> bool
	{
		return hash == other.hash;
	}
	SH_CORE_API auto Name::operator!=(const Name& other) const -> bool
	{
		return hash != other.hash;
	}
	SH_CORE_API auto Name::operator==(const std::string& str) const -> bool
	{
		return hash == core::Util::ConstexprHash(str);
	}
	SH_CORE_API auto Name::operator!=(const std::string& str) const -> bool
	{
		return hash != core::Util::ConstexprHash(str);
	}
	SH_CORE_API auto Name::ToString() const -> const std::string&
	{
		lock.Lock();
		const std::string& result = map[hash];
		lock.UnLock();
		return result;
	}
}//namespace