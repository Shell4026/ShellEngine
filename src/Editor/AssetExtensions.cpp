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
		{".json", Type::Text},
		{".font", Type::Font},
		{".ttf", Type::Binary}
	};

	SH_EDITOR_API void AssetExtensions::AddExtension(const std::string& ext, Type type)
	{
		extensions.insert({ ext, type });
	}
	SH_EDITOR_API auto AssetExtensions::CheckType(const std::string& extension) -> AssetExtensions::Type
	{
		auto it = extensions.find(extension);
		if (it == extensions.end())
			return Type::None;
		return it->second;
	}
	SH_EDITOR_API auto AssetExtensions::ToString(Type type) -> const char*
	{
		switch (type)
		{
		case Type::None:
			return "None";
		case Type::Model:
			return "Model";
		case Type::Texture:
			return "Texture";
		case Type::Material:
			return "Material";
		case Type::World:
			return "World";
		case Type::Shader:
			return "Shader";
		case Type::Prefab:
			return "Prefab";
		case Type::Text:
			return "Text";
		case Type::Font:
			return "Font";
		case Type::Binary:
			return "Binary";
		}
		return "None";
	}
}//namespace