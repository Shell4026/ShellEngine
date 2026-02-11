#pragma once
#include "Editor/Export.h"
#include "CustomInspector.h"

#include "Render/Material.h"
#include "Render/Texture.h"

#include "Game/GameObject.h"
#include "Game/Component/Render/Camera.h"
#include "Game/TextObject.h"
#include "Game/GUITexture.h"

#include <unordered_map>
namespace sh::editor
{
	class GameObjectInspector : public CustomInspector
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

	class MaterialInspector : public CustomInspector
	{
		INSPECTOR(MaterialInspector, render::Material)
	public:
		SH_EDITOR_API void RenderUI(void* obj, int idx) override;
	};

	class TextureInspector : public CustomInspector
	{
		INSPECTOR(TextureInspector, render::Texture)
	public:
		SH_EDITOR_API TextureInspector();
		SH_EDITOR_API ~TextureInspector();
		SH_EDITOR_API void RenderUI(void* obj, int idx) override;
	private:
		const render::Texture* lastTex = nullptr;
		game::GUITexture* previewTex = nullptr;
	};

	class CameraInspector : public CustomInspector
	{
		INSPECTOR(CameraInspector, game::Camera)
	public:
		SH_EDITOR_API void RenderUI(void* obj, int idx) override;
	};

	class TextInspector : public CustomInspector
	{
		INSPECTOR(TextInspector, game::TextObject)
	public:
		SH_EDITOR_API void RenderUI(void* obj, int idx) override;
	};
}//namespace