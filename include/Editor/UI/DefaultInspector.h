#pragma once
#include "Export.h"
#include "CustomInspector.h"

#include "Render/Material.h"
#include "Render/Texture.h"

#include "Game/GameObject.h"
#include "Game/Component/Camera.h"

#include <unordered_map>
namespace sh::editor
{
	class GameObjectInspector : public ICustomInspector
	{
		INSPECTOR(GameObjectInspector, game::GameObject)
	public:
		SH_EDITOR_API void RenderUI(void* obj, int idx) override;
	private:
		void RenderAddComponent(game::GameObject& gameObject);
		auto GetComponentGroupAndName(std::string_view fullname) -> std::pair<std::string, std::string>;
	private:
		std::unordered_map<std::string, std::vector<std::string>> componentItems;
		bool bAddComponent = false;
	};

	class MaterialInspector : public ICustomInspector
	{
		INSPECTOR(MaterialInspector, render::Material)
	public:
		SH_EDITOR_API void RenderUI(void* obj, int idx) override;
	};

	class TextureInspector : public ICustomInspector
	{
		INSPECTOR(TextureInspector, render::Texture)
	public:
		SH_EDITOR_API void RenderUI(void* obj, int idx) override;
	};

	class CameraInspector : public ICustomInspector
	{
		INSPECTOR(CameraInspector, game::Camera)
	public:
		SH_EDITOR_API void RenderUI(void* obj, int idx) override;
	};
}//namespace