#include "ShaderLoader.h"

#include "Core/FileSystem.h"
#include "Core/Logger.h"
#include "Core/ExecuteProcess.h"

#include "Render/Shader.h"
#include "Render/ShaderPassBuilder.h"
#include "Render/ShaderParser.h"
#include "Render/ShaderGenerator.h"
#include "Render/ShaderCreateInfo.h"

#include <string>

namespace sh::editor
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

	SH_EDITOR_API void ShaderLoader::SetCachePath(const std::filesystem::path& path)
	{
		cachePath = path;
	}

	SH_EDITOR_API auto ShaderLoader::GetCachePath() const -> const std::filesystem::path&
	{
		return cachePath;
	}

	SH_EDITOR_API auto ShaderLoader::LoadShader(const std::filesystem::path& path) -> render::Shader*
	{
		auto src = core::FileSystem::LoadText(path);
		if (!src.has_value())
		{
			SH_ERROR_FORMAT("Can't load file: {}", path.string());
			return nullptr;
		}
		render::ShaderLexer lexer{};
		render::ShaderParser parser{};
		render::ShaderAST::ShaderNode shaderNode = parser.Parse(lexer.Lex(src.value()));
		
		render::ShaderCreateInfo shaderCI{};
		shaderCI.SetShaderNode(shaderNode);
		for (auto& pass : shaderNode.passes)
		{
			auto shaderPaths = render::ShaderGenerator::GenerateShaderFile(shaderNode.shaderName, pass, cachePath);

			for (auto& shaderPath : shaderPaths)
			{
				std::string name = shaderPath.filename().string() + ".spv";
				std::filesystem::path spirvPath = (shaderPath.parent_path() / name);

				auto stageType = render::ShaderPassBuilder::shaderType::Vertex;
				if (shaderPath.extension() == ".frag")
					stageType = render::ShaderPassBuilder::shaderType::Fragment;

				// 컴파일
				std::vector<std::string> args = { shaderPath.string(), "-o", spirvPath.string() };
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
			auto shaderPass = passBuilder->Build(pass);
			if (shaderPass == nullptr)
				return nullptr;

			shaderCI.AddShaderPass(std::move(shaderPass));
		}
		// 셰이더 생성
		auto shader = core::SObject::Create<render::Shader>(std::move(shaderCI));
		return shader;
	}
}//namespace