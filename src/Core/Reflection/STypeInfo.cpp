#include "Reflection/STypeInfo.hpp"

namespace sh::core::reflection
{
	std::unordered_map<std::size_t, STypeInfo*> STypes::types{};

	SH_CORE_API auto STypeInfo::AddProperty(const Property& prop) -> Property*
	{
		if (GetProperty(prop.GetName()) != nullptr)
			return nullptr;
		properties.push_back(std::make_unique<Property>(prop));
		auto propPtr = properties.back().get();
		if (prop.isSObjectPointer)
			sobjPtrs.push_back(propPtr);
		else if (prop.isSObjectPointerContainer)
			sobjPtrContainers.push_back(propPtr);
		return propPtr;
	}
	SH_CORE_API auto STypeInfo::operator==(const STypeInfo& other) const -> bool
	{
		if (this == &other)
			return true;
		// 다른 dll 영역에 올라갈 경우 같은 클래스여도 주소가 다르다. 따라서 Hash값으로 비교한다.
		return this->hash == other.hash;
	}
	SH_CORE_API auto STypeInfo::operator!=(const STypeInfo& other) const -> bool
	{
		return !operator==(other);
	}

	SH_CORE_API auto STypeInfo::GetSuper() const -> STypeInfo*
	{
		return super;
	}

	SH_CORE_API auto STypeInfo::IsChildOf(const STypeInfo& other) const -> bool
	{
		if (operator==(other))
			return true;

		const STypeInfo* super = GetSuper();
		while (super != nullptr)
		{
			if (super->operator==(other))
				return true;
			super = super->GetSuper();
		}
		return false;
	}
	SH_CORE_API auto STypeInfo::GetProperty(const core::Name& name) const -> Property*
	{
		for (auto& prop : properties)
		{
			if (prop->GetName() == name)
				return prop.get();
		}
		return nullptr;
	}
	SH_CORE_API auto STypeInfo::GetProperty(std::string_view name) const -> Property*
	{
		return GetProperty(core::Name{ name });
	}
	SH_CORE_API auto STypeInfo::GetProperties() const -> const std::vector<std::unique_ptr<Property>>&
	{
		return properties;
	}
	SH_CORE_API auto STypeInfo::GetSObjectPtrProperties() const -> const std::vector<Property*>&
	{
		return sobjPtrs;
	}
	SH_CORE_API auto STypeInfo::GetSObjectPtrContainerProperties() const -> const std::vector<Property*>&
	{
		return sobjPtrContainers;
	}

}//namespace