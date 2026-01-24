#include "UI/FontGeneratorUI.h"
#include "Meta.h"

#include "Core/FileSystem.h"
#include "Core/Logger.h"

#include "Game/Asset/FontGenerator.h"

#include "External/imgui/misc/cpp/imgui_stdlib.h"

namespace sh::editor
{
	FontGeneratorUI::FontGeneratorUI(const render::IRenderContext& ctx) :
		renderCtx(ctx)
	{
		explorer.SetExtensionFilter({ ".ttf", ".otf" });
	}
	SH_EDITOR_API void FontGeneratorUI::SetAssetPath(const std::filesystem::path& assetPath)
	{
		this->assetPath = assetPath;
		explorer.SetCurrentPath(assetPath);
	}
	SH_EDITOR_API void FontGeneratorUI::Clear()
	{
		if (!fontPath.empty())
			fontPath.clear();
		if (!fontData.empty())
			fontData.clear();
		if (!str.empty())
			str.clear();
		fontSize = 32.f;
		padding = 2;
		atlasWidth = 1024;
		atlasHeight = 1024;
	}
	SH_EDITOR_API void FontGeneratorUI::Open()
	{
		bShow = true;
	}
	SH_EDITOR_API void FontGeneratorUI::Render()
	{
		if (!bShow)
		{
			Clear();
			return;
		}

		ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_::ImGuiCond_FirstUseEver);
		if (ImGui::Begin("FontGeneratorUI", &bShow))
		{
			if (!fontPath.empty())
			{
				ImGui::Text(fontPath.u8string().c_str());
				ImGui::SameLine();
			}
			if (ImGui::Button("Open Font File"))
			{
				explorer.PushCallbackQueue(
					[this](const std::filesystem::path& path)
					{
						auto opt = core::FileSystem::LoadBinary(path);
						if (!opt.has_value())
							return;

						fontPath = path;
						fontData = std::move(opt.value());
					}
				);
				explorer.Open(ExplorerUI::OpenMode::Select);
			}
			if (!fontData.empty())
			{
				ImGui::Separator();

				ImGui::Text("Font size");
				ImGui::InputFloat("##fontSize", &fontSize);

				ImGui::Text("Padding");
				ImGui::InputInt("##padding", &padding);
				if (padding <= 0)
					padding = 1;

				ImGui::Text("Atlas size");
				int size[2] = { atlasWidth, atlasHeight };
				ImGui::InputInt2("##padding", size);
				if (size[0] <= 0)
					size[0] = 1;
				if (size[1] <= 0)
					size[1] = 1;
				atlasWidth = size[0];
				atlasHeight = size[1];

				ImGui::Text("Characters");
				ImGui::InputText("##fontstrs", &str);

				if (ImGui::Button("Generate font data"))
				{
					explorer.PushCallbackQueue(
						[this](const std::filesystem::path& path)
						{
							game::FontGenerator::Options option{};
							option.fontSize = fontSize;
							option.padding = padding;
							option.atlasW = atlasWidth;
							option.atlasH = atlasHeight;
							option.bAllowMultiPage = true;

							render::Font* font = game::FontGenerator::GenerateFont(renderCtx, fontData, str, option);
							if (font == nullptr)
							{
								SH_ERROR_FORMAT("Failed to generate font!: {}", fontPath.u8string());
								return;
							}
							const std::string fileName = path.stem().u8string();

							font->SetName(fileName);

							const std::filesystem::path exportedFontPath = path.parent_path() / std::filesystem::u8path(fileName + ".font");
							std::vector<std::filesystem::path> exportedAtlasPaths;

							if (!core::FileSystem::SaveText(font->Serialize(), exportedFontPath))
							{
								SH_ERROR_FORMAT("Failed to export font: {}", exportedFontPath.u8string());
								return;
							}

							int idx = 0;
							for (auto tex : font->GetAtlases())
							{
								std::filesystem::path exportPath = path.parent_path() / std::filesystem::u8path(fileName + std::to_string(idx++) + ".png");
								tex->SetName(exportPath.stem().u8string());
								tex->ExportToPNG(exportPath);
								exportedAtlasPaths.push_back(std::move(exportPath));
							}

							// 에셋 폴더내에 생성했다면 메타 파일 생성
							const std::filesystem::path relativePath = std::filesystem::relative(path, assetPath);
							if (!relativePath.empty())
							{
								const auto fontMetaDir = Meta::CreateMetaDirectory(exportedFontPath);
								if (!std::filesystem::exists(fontMetaDir))
								{
									Meta meta{};
									meta.Save(*font, Meta::CreateMetaDirectory(exportedFontPath));
								}
								int idx = 0;
								for (auto tex : font->GetAtlases())
								{
									const auto atlasMetaDir = Meta::CreateMetaDirectory(exportedAtlasPaths[idx++]);
									if (!std::filesystem::exists(atlasMetaDir))
									{
										Meta atlasMeta{};
										atlasMeta.Save(*tex, atlasMetaDir);
									}
								}
							}
						}
					);
					explorer.ResetSelected();
					explorer.Open(ExplorerUI::OpenMode::Create);
				}
			}
		}
		ImGui::End();

		explorer.Render();
	}
}//namespace