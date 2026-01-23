#pragma once
#include "Export.h"

#include <unordered_map>
#include <string>
namespace sh::editor
{
	class AssetExtensions
	{
	public:
		enum class Type
		{
			None,
			Model,
			Texture,
			Material,
			World,
			Shader,
			Prefab,
			Text,
			Font
		};
	private:
		static std::unordered_map<std::string, Type> extensions;
	public:
		SH_EDITOR_API static auto CheckType(const std::string& extension) -> Type;
	};
}//namespace