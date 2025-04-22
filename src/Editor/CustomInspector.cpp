#include "UI/CustomInspector.h"

namespace sh::editor
{
	auto sh::editor::CustomInspectorManager::GetCustomInspector(const core::reflection::STypeInfo* type) const -> ICustomInspector*
	{
		auto it = map.find(type);
		if (it == map.end())
			return nullptr;
		return it->second.get();
	}
}