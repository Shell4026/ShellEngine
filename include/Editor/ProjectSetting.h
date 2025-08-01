﻿#pragma once
#include "Export.h"

#include "Core/ISerializable.h"

#include <filesystem>
#include <string>
namespace sh::editor
{
	class ProjectSetting : public core::ISerializable
	{
	private:
		int version;
	public:
		std::filesystem::path startingWorldPath;
	public:
		SH_EDITOR_API ProjectSetting();

		SH_EDITOR_API auto Serialize() const -> core::Json override;
		SH_EDITOR_API void Deserialize(const core::Json& json) override;
	};
}//namespace