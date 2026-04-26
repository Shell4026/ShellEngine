#include "Asset/ComputeShaderLoader.h"
#include "Asset/ComputeShaderAsset.h"

#include "Core/FileSystem.h"
#include "Core/Logger.h"
#include "Core/ExecuteProcess.h"

#include "Render/ComputeShader.h"
#include "Render/ComputeShaderCreateInfo.h"
#include "Render/ComputeShaderLexer.h"
#include "Render/ComputeShaderParser.h"

#include <fmt/core.h>

#include <cctype>
#include <string>
#include <filesystem>
namespace sh::game
{
	static auto ReplaceSpaceString(const std::string& str) -> std::string
	{
		std::string result;
		result.reserve(str.size());
		for (char c : str)
		{
			if (std::isalnum(static_cast<unsigned char>(c)))
				result.push_back(c);
			else
				result.push_back('_');
		}
		return result;
	}

	ComputeShaderLoader::ComputeShaderLoader()
	{
		cachePath = std::filesystem::current_path() / "cache";
		if (!std::filesystem::exists(cachePath))
			std::filesystem::create_directory(cachePath);
	}
	ComputeShaderLoader::~ComputeShaderLoader() = default;

	SH_GAME_API auto ComputeShaderLoader::Load(const std::filesystem::path& path) const -> core::SObject*
	{
		std::optional<std::string> srcOpt = core::FileSystem::LoadText(path);
		if (!srcOpt.has_value())
		{
			SH_ERROR_FORMAT("Can't load file: {}", path.string());
			return nullptr;
		}
		if (!std::filesystem::exists(cachePath))
			std::filesystem::create_directory(cachePath);

		render::ComputeShaderLexer lexer{};
		render::ComputeShaderParser parser{};

		render::ComputeShaderCreateInfo csCI{};
		try
		{
			csCI.shaderNode = parser.Parse(lexer.Lex(srcOpt.value()));
		}
		catch (const render::ComputeShaderParserException& e)
		{
			SH_ERROR_FORMAT("Compute shader parsing error: {}", e.what());
			return nullptr;
		}
		catch (const render::ComputeShaderLexerException& e)
		{
			SH_ERROR_FORMAT("Compute shader lexing error: {}", e.what());
			return nullptr;
		}

		// 셰이더 이름이 비어있으면 파일명을 사용
		if (csCI.shaderNode.shaderName.empty())
			csCI.shaderNode.shaderName = path.stem().string();

		// GLSL 파일 작성
		const std::string fileName = ReplaceSpaceString(csCI.shaderNode.shaderName) + ".comp";
		const std::filesystem::path glslPath = cachePath / fileName;
		if (!core::FileSystem::SaveText(csCI.shaderNode.code, glslPath))
		{
			SH_ERROR_FORMAT("Can't write compute shader source: {}", glslPath.string());
			return nullptr;
		}

		// SPIR-V 컴파일
		const std::filesystem::path spirvPath = cachePath / (fileName + ".spv");

		std::string output;
		std::string compiler = "glslc";
#if _WIN32
		std::vector<std::string> args = {
			"-fshader-stage=compute",
			fmt::format("\"{}\"", glslPath.u8string()),
			"-o",
			fmt::format("\"{}\"", spirvPath.u8string())
		};
		compiler += ".exe";
#else
		std::vector<std::string> args = {
			"-fshader-stage=compute",
			fmt::format("{}", glslPath.u8string()),
			"-o",
			fmt::format("{}", spirvPath.u8string())
		};
#endif
		if (!core::ExecuteProcess::Execute(compiler, args, output))
		{
			SH_ERROR_FORMAT("Compute shader compile error: {}", output);
			return nullptr;
		}

		// SPIR-V 로드
		auto spirvOpt = core::FileSystem::LoadBinary(spirvPath);
		if (!spirvOpt.has_value())
		{
			SH_ERROR_FORMAT("Compute shader spirv load error: {}", spirvPath.string());
			return nullptr;
		}
		csCI.spirv = std::move(spirvOpt.value());

		render::ComputeShader* const shader = core::SObject::Create<render::ComputeShader>(std::move(csCI));
		return shader;
	}

	SH_GAME_API auto ComputeShaderLoader::Load(const core::Asset& asset) const -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), ASSET_NAME) != 0)
		{
			SH_ERROR_FORMAT("Asset({}) is not a compute shader!", asset.GetAssetUUID().ToString());
			return nullptr;
		}
		const game::ComputeShaderAsset& csAsset = static_cast<const game::ComputeShaderAsset&>(asset);
		const core::Json& shaderObjJson = csAsset.GetShaderObjectJson();

		if (!shaderObjJson.contains("computeShader"))
			return nullptr;

		const core::Json& csJson = shaderObjJson["computeShader"];

		render::ComputeShaderCreateInfo csCI{};
		if (!csJson.contains("AST"))
			return nullptr;

		csCI.shaderNode.Deserialize(csJson["AST"]);
		if (csJson.contains("spirv"))
			csCI.spirv = csJson["spirv"].get<std::vector<uint8_t>>();

		if (core::SObject* const oldObj = core::SObjectManager::GetInstance()->GetSObject(asset.GetAssetUUID()); oldObj != nullptr)
		{
			render::ComputeShader* const oldShaderPtr = core::reflection::Cast<render::ComputeShader>(oldObj);
			if (oldShaderPtr == nullptr)
				return nullptr;
			oldShaderPtr->Deserialize(shaderObjJson);
			return oldShaderPtr;
		}
		render::ComputeShader* const shader = core::SObject::Create<render::ComputeShader>(std::move(csCI));
		shader->Deserialize(shaderObjJson);

		return shader;
	}
}//namespace
