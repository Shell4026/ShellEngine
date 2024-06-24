#include "Reflection.hpp"

namespace sh::core::reflection
{
	auto TypeInfo::GetName() const -> std::string_view
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

	void TypeInfo::AddSObjectPtrProperty(Property* prop)
	{
		pointers.push_back(prop);
	}
	auto TypeInfo::GetSObjectPtrProperties() const -> const std::vector<Property*>&
	{
		return pointers;
	}

	void TypeInfo::AddSObjectContainerProperty(Property* prop)
	{
		containers.push_back(prop);
	}
	auto TypeInfo::GetSObjectContainerProperties() const -> const std::vector<Property*>&
	{
		return containers;
	}

	auto PropertyDataBase::GetTypeName() const -> std::string_view
	{
		return typeName;
	}

	Property::Property(PropertyDataBase * data, const char* name, bool isContainer, uint32_t containerNestedLevel) :
		data(data), name(name), isContainer(isContainer), containerNestedLevel(containerNestedLevel),
		isConst(data->isConst), bVisible(data->bVisible)
	{

	}

	auto Property::GetName() const -> const char*
	{
		return name;
	}

	auto Property::GetTypeName() const -> std::string_view
	{
		return data->GetTypeName();
	}

	auto Property::Begin(SObject* sobject) -> PropertyIterator
	{
		return data->Begin(sobject);
	}

	auto Property::End(SObject* sobject) -> PropertyIterator
	{
		return data->End(sobject);
	}

	PropertyIterator::PropertyIterator()
	{
	}

	PropertyIterator::PropertyIterator(PropertyIterator&& other) noexcept :
		iteratorData(std::move(other.iteratorData))
	{
	}

	PropertyIterator::PropertyIterator(std::unique_ptr<IPropertyIteratorBase>&& iteratorData) :
		iteratorData(std::move(iteratorData))
	{
	}

	auto PropertyIterator::operator==(const PropertyIterator & other) -> bool
	{
		return *iteratorData.get() == *other.iteratorData.get();
	}

	auto PropertyIterator::operator!=(const PropertyIterator& other) -> bool
	{
		return *iteratorData.get() != *other.iteratorData.get();
	}

	auto PropertyIterator::operator++() -> PropertyIterator&
	{
		++(*iteratorData.get());
		return *this;
	}

	auto PropertyIterator::GetTypeName() const -> std::string_view
	{
		return iteratorData->typeName;
	}

	auto PropertyIterator::GetNestedBegin() -> PropertyIterator
	{
		return iteratorData->GetNestedBegin();
	}

	auto PropertyIterator::GetNestedEnd() -> PropertyIterator
	{
		return iteratorData->GetNestedEnd();
	}

	auto PropertyIterator::IsPair() const -> bool
	{
		return iteratorData->IsPair();
	}
;}