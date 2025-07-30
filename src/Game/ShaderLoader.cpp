#include "ShaderLoader.h"
#include "ShaderAsset.h"

#include "Core/FileSystem.h"
#include "Core/Logger.h"
#include "Core/ExecuteProcess.h"

#include "Render/Shader.h"
#include "Render/ShaderPassBuilder.h"
#include "Render/ShaderParser.h"
#include "Render/ShaderGenerator.h"
#include "Render/ShaderCreateInfo.h"

#include <string>
#include <filesystem>
namespace sh::game
{
	namespace fs = std::filesystem;
	ShaderLoader::ShaderLoader(render::ShaderPassBuilder* builder) :
		passBuilder(builder)
	{
		cachePath = fs::current_path() / "cache";
		if (!fs::exists(cachePath))
			fs::create_directory(cachePath);
	}
	ShaderLoader::~ShaderLoader()
	{
	}

	SH_GAME_API void ShaderLoader::SetCachePath(const std::filesystem::path& path)
	{
		cachePath = path;
	}

	SH_GAME_API auto ShaderLoader::GetCachePath() const -> const std::filesystem::path&
	{
		return cachePath;
	}

	SH_GAME_API auto ShaderLoader::Load(const std::filesystem::path& path) -> core::SObject*
	{
		auto src = core::FileSystem::LoadText(path);
		if (!src.has_value())
		{
			SH_ERROR_FORMAT("Can't load file: {}", path.string());
			return nullptr;
		}
		if (!std::filesystem::exists(cachePath))
			std::filesystem::create_directory(cachePath);

		render::ShaderLexer lexer{};
		render::ShaderParser parser{};

		render::ShaderCreateInfo shaderCI{};
		shaderCI.shaderNode = parser.Parse(lexer.Lex(src.value()));

		for (auto& passNode : shaderCI.shaderNode.passes)
		{
			auto shaderPaths = render::ShaderGenerator::GenerateShaderFile(shaderCI.shaderNode.shaderName, passNode, cachePath);

			for (auto& shaderPath : shaderPaths)
			{
				std::string name = shaderPath.filename().string() + ".spv";
				std::filesystem::path spirvPath = (shaderPath.parent_path() / name);

				auto stageType = render::ShaderPassBuilder::shaderType::Vertex;
				if (shaderPath.extension() == ".frag")
					stageType = render::ShaderPassBuilder::shaderType::Fragment;

				// 컴파일
				std::vector<std::string> args = { fmt::format("\"{}\"", shaderPath.u8string()), "-o", fmt::format("\"{}\"", spirvPath.u8string()) };
				std::string output;
				std::string compiler = "glslc";
#if _WIN32
				compiler += ".exe";
#endif
				if (!core::ExecuteProcess::Execute(compiler, args, output))
				{
					SH_ERROR_FORMAT("Shader compile error: {}", output);
					return nullptr;
				}
				// spir-v 불러오기
				auto spirv = core::FileSystem::LoadBinary(spirvPath);
				if (!spirv.has_value())
				{
					SH_ERROR_FORMAT("Temp shader load error: {}", spirvPath.string());
					return nullptr;
				}
				passBuilder->SetData(stageType, std::move(spirv.value()));
			}

			// 패스 생성
			render::ShaderPass* shaderPass = passBuilder->Build(passNode);
			if (shaderPass == nullptr)
				return nullptr;

			shaderCI.passes.push_back(shaderPass);
		}
		// 셰이더 생성
		auto shader = core::SObject::Create<render::Shader>(std::move(shaderCI));
		return shader;
	}

	SH_GAME_API auto ShaderLoader::Load(const core::Asset& asset) -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), ASSET_NAME) != 0)
		{
			SH_ERROR_FORMAT("Asset({}) is not a shader!", asset.GetAssetUUID().ToString());
			return nullptr;
		}
		const auto& shaderAsset = static_cast<const game::ShaderAsset&>(asset);
		const core::Json& shaderObjJson = shaderAsset.GetShaderObjectJson();

		if (!shaderObjJson.contains("shader"))
			return nullptr;

		const core::Json& shaderJson = shaderObjJson["shader"];

		render::ShaderCreateInfo shaderCI{};
		if (!shaderJson.contains("AST"))
			return nullptr;

		shaderCI.shaderNode.Deserialize(shaderJson["AST"]);

		if (shaderCI.shaderNode.passes.size() != shaderJson["passes"].size())
			return nullptr;

		for (int i = 0; i < shaderCI.shaderNode.passes.size(); ++i)
		{
			const auto& passNode = shaderCI.shaderNode.passes[i];
			const core::Json& shaderPassObjJson = *(shaderJson["passes"].begin() + i);
			const core::Json& shaderPassJson = shaderPassObjJson["shaderPass"];

			if (shaderPassJson.contains("vertShaderData"))
				passBuilder->SetData(render::ShaderPassBuilder::shaderType::Vertex, shaderPassJson["vertShaderData"].get<std::vector<uint8_t>>());
			if (shaderPassJson.contains("fragShaderData"))
				passBuilder->SetData(render::ShaderPassBuilder::shaderType::Fragment, shaderPassJson["fragShaderData"].get<std::vector<uint8_t>>());

			render::ShaderPass* shaderPass = passBuilder->Build(passNode);
			if (shaderPass == nullptr)
				return nullptr;
			shaderPass->Deserialize(shaderPassObjJson);

			shaderCI.passes.push_back(shaderPass);
		}
		auto shader = core::SObject::Create<render::Shader>(std::move(shaderCI));
		shader->Deserialize(shaderObjJson);

		return shader;
	}

	SH_GAME_API auto ShaderLoader::GetAssetName() const -> const char*
	{
		return "shad";
	}
}//namespace