#include "ShaderAttribute.h"

namespace sh::render
{
	ShaderAttributeBase::ShaderAttributeBase(std::string_view name, bool isInteger) :
		name(attributeName),
		attributeName(name),
		isInteger(isInteger)
	{}

	ShaderAttributeBase::ShaderAttributeBase(const ShaderAttributeBase& other) :
		name(attributeName),
		attributeName(other.attributeName),
		isInteger(other.isInteger)
	{}

	ShaderAttributeBase::ShaderAttributeBase(ShaderAttributeBase&& other) noexcept :
		name(attributeName),
		attributeName(std::move(other.attributeName)),
		isInteger(other.isInteger)
	{
	}
}