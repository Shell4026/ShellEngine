﻿#include "Reflection.hpp"

namespace sh::core::reflection
{
	auto TypeInfo::GetName() const -> const char*
	{
		return name;
	}

	auto TypeInfo::GetSuper() const -> const TypeInfo*
	{
		return super;
	}

	bool TypeInfo::IsA(const TypeInfo& other) const
	{
		if (this == &other)
			return true;

		//다른 dll 영역에 올라갈 경우 같은 클래스여도 주소가 다르다. 따라서 Hash값으로 비교한다.
		return this->hash == other.hash;
	}

	bool TypeInfo::operator==(const TypeInfo& other) const
	{
		return IsA(other);
	}

	bool TypeInfo::IsChildOf(const TypeInfo& other) const
	{
		if (IsA(other))
			return true;

		const TypeInfo* super = GetSuper();
		while (super != nullptr)
		{
			if (super->IsA(other))
				return true;
			super = super->GetSuper();
		}
		return false;
	}

	auto TypeInfo::AddProperty(const std::string& name, const Property& prop) -> Property*
	{
		auto it = properties.insert({ name, prop });
		if (!it.second)
			return nullptr;
		return &it.first->second;
	}

	void TypeInfo::AddPointerProperty(Property* prop)
	{
		return pointers.push_back(prop);
	}

	auto TypeInfo::GetProperty(const std::string& name) -> Property*
	{
		auto it = properties.find(name);
		if (it == properties.end())
			return nullptr;
		return &it->second;
	}

	auto TypeInfo::GetProperties() const -> const std::map<std::string, Property>&
	{
		return properties;
	}

	auto TypeInfo::GetSObjectPtrProperties() const -> const std::vector<Property*>&
	{
		return pointers;
	}
;}