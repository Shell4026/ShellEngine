#include "AssetExtensions.h"

namespace sh::editor
{
	std::unordered_map<std::string, AssetExtensions::Type> AssetExtensions::extensions
	{ 
		{".obj", Type::Model},
		{".glb", Type::Model},
		{".png", Type::Texture},
		{".jpg", Type::Texture},
		{".mat", Type::Material},
		{".world", Type::World},
		{".shader", Type::Shader},
		{".prefab", Type::Prefab},
		{".txt", Type::Text},
		{".json", Type::Text}
	};

	SH_EDITOR_API auto AssetExtensions::CheckType(const std::string& extension) -> AssetExtensions::Type
	{
		auto it = extensions.find(extension);
		if (it == extensions.end())
			return Type::None;
		return it->second;
	}
}//namespace