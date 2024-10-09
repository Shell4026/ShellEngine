#include "pch.h"
#include "ShaderAttribute.h"

namespace sh::render
{
	ShaderAttributeBase::ShaderAttributeBase(std::string_view name, bool isInteger) :
		name(attributeName),
		typeName(mTypeName),

		attributeName(name),
		isInteger(isInteger)
	{}

	ShaderAttributeBase::ShaderAttributeBase(ShaderAttributeBase&& other) noexcept :
		name(attributeName),
		typeName(std::move(mTypeName)),

		attributeName(std::move(other.attributeName)),
		isInteger(other.isInteger)
	{
	}
}