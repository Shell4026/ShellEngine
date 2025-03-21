#include "Name.h"

namespace sh::core
{
	std::unordered_map<std::string, int> Name::dic{};
	std::mutex Name::mu{};

	Name::Name(const std::string& str)
	{
		mu.lock();
		auto it = dic.find(str);
		if (it == dic.end())
			ptr = &dic.insert_or_assign(str, 1).first->first;
		else
		{
			ptr = &it->first;
			++it->second;
		}
		mu.unlock();
;	}
	Name::Name(const char* _str)
	{
		mu.lock();
		std::string str(_str);
		auto it = dic.find(str);
		if (it == dic.end())
			ptr = &dic.insert_or_assign(std::move(str), 1).first->first;
		else
		{
			ptr = &it->first;
			++it->second;
		}
		mu.unlock();
	}
	Name::~Name()
	{
		mu.lock();
		auto it = dic.find(*ptr);
		if (--it->second == 0)
			dic.erase(it);
		mu.unlock();
	}
	SH_CORE_API auto Name::operator==(const Name& other) const -> bool
	{
		return ptr == other.ptr;
	}
	SH_CORE_API auto Name::operator==(const std::string& str) const -> bool
	{
		auto it = dic.find(str);
		if (it == dic.end())
			return false;
		return ptr == &it->first;
	}
	SH_CORE_API auto Name::operator!=(const Name& other) const -> bool
	{
		return  ptr != other.ptr;
	}
	SH_CORE_API auto Name::operator!=(const std::string& str) const -> bool
	{
		return !operator==(str);
	}
	SH_CORE_API Name::operator const std::string& () const
	{
		return *ptr;
	}
	SH_CORE_API auto Name::GetString() const -> const std::string&
	{
		return *ptr;
	}
}//namespace