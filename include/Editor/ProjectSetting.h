#pragma once
#include "Export.h"

#include "Core/ISerializable.h"
#include "Core/SContainer.hpp"

#include <filesystem>
#include <string>

#include "Game/World.h"

namespace sh::editor
{
	class ProjectSetting : public core::ISerializable
	{
	public:
		SH_EDITOR_API ProjectSetting();
		SH_EDITOR_API ~ProjectSetting();

		SH_EDITOR_API auto Serialize() const -> core::Json override;
		SH_EDITOR_API void Deserialize(const core::Json& json) override;

		SH_EDITOR_API void Save(const std::filesystem::path& path);
		SH_EDITOR_API void Load(const std::filesystem::path& path);

		SH_EDITOR_API void RenderUI(bool& bOpen, const std::filesystem::path& rootPath);
	public:
		core::SObjWeakPtr<game::World> startingWorld = nullptr;
	private:
		int version;
	};
}//namespace