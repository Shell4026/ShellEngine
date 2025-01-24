#include "ShaderLoader.h"

#include "Core/FileSystem.h"
#include "Core/Logger.h"
#include "Core/ExecuteProcess.h"
#include "Core/Util.h"

#include "Render/ShaderPassBuilder.h"
#include "Render/ShaderParser.h"
#include "Render/ShaderGenerator.h"

#include <fmt/core.h>

#include <string>

namespace sh::game
{
	static void FillPassAttributes(const render::ShaderAST::PassNode& passNode, render::ShaderPass& pass)
	{
		for (auto& stage : passNode.stages)
		{
			auto stageType = (stage.type == render::ShaderAST::StageType::Vertex) ? 
				render::ShaderPass::ShaderStage::Vertex:
				render::ShaderPass::ShaderStage::Fragment;

			if (stage.type == render::ShaderAST::StageType::Vertex)
			{
				for (auto& in : stage.in)
				{
					switch (in.var.type)
					{
					case render::ShaderAST::VariableType::Vec4:  pass.AddAttribute<glm::vec4>(in.var.name, in.binding); break;
					case render::ShaderAST::VariableType::Vec3:  pass.AddAttribute<glm::vec3>(in.var.name, in.binding); break;
					case render::ShaderAST::VariableType::Vec2:  pass.AddAttribute<glm::vec2>(in.var.name, in.binding); break;
					case render::ShaderAST::VariableType::Mat4:  pass.AddAttribute<glm::mat4>(in.var.name, in.binding); break;
					case render::ShaderAST::VariableType::Mat3:  pass.AddAttribute<glm::mat3>(in.var.name, in.binding); break;
					case render::ShaderAST::VariableType::Float: pass.AddAttribute<    float>(in.var.name, in.binding); break;
					case render::ShaderAST::VariableType::Int:   pass.AddAttribute<      int>(in.var.name, in.binding); break;
					}
				}
			}
			for (auto& uniform : stage.uniforms)
			{
				auto uniformType = (uniform.set == 0) ?
					render::ShaderPass::UniformType::Object : 
					render::ShaderPass::UniformType::Material;

				if (!uniform.bSampler)
				{
					for (auto& var : uniform.vars)
					{
						switch (var.type)
						{
						case render::ShaderAST::VariableType::Vec4:  pass.AddUniformArray<glm::vec4>(var.name, var.size, uniformType, uniform.binding, stageType); break;
						case render::ShaderAST::VariableType::Vec3:  pass.AddUniformArray<glm::vec3>(var.name, var.size, uniformType, uniform.binding, stageType); break;
						case render::ShaderAST::VariableType::Vec2:  pass.AddUniformArray<glm::vec2>(var.name, var.size, uniformType, uniform.binding, stageType); break;
						case render::ShaderAST::VariableType::Mat4:  pass.AddUniformArray<glm::mat4>(var.name, var.size, uniformType, uniform.binding, stageType); break;
						case render::ShaderAST::VariableType::Mat3:  pass.AddUniformArray<glm::mat3>(var.name, var.size, uniformType, uniform.binding, stageType); break;
						case render::ShaderAST::VariableType::Float: pass.AddUniformArray<    float>(var.name, var.size, uniformType, uniform.binding, stageType); break;
						case render::ShaderAST::VariableType::Int:   pass.AddUniformArray<      int>(var.name, var.size, uniformType, uniform.binding, stageType); break;
						}
					}
				}
				else
					pass.AddUniform<render::Texture>(uniform.name, uniformType, uniform.binding, stageType);
			}
		}
	}

	ShaderLoader::ShaderLoader(render::ShaderPassBuilder* builder) :
		builder(builder)
	{
	}
	ShaderLoader::~ShaderLoader()
	{
	}

	SH_GAME_API auto ShaderLoader::LoadShader(const std::filesystem::path& path) -> render::Shader*
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

		render::Shader* shader = core::SObject::Create<render::Shader>();
		for (auto& pass : shaderNode.passes)
		{
			auto shaderPaths = render::ShaderGenerator::GenerateShaderFile(shaderNode.shaderName, pass, path.parent_path());

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
				builder->SetData(stageType, std::move(spirv.value()));
			}

			auto shaderPass = builder->Build();
			if (shaderPass == nullptr)
				return nullptr;

			shaderPass->SetStencilState(pass.stencil);
			FillPassAttributes(pass, *shaderPass.get());
			shaderPass->Build();
			shader->AddPass(std::move(shaderPass));
		}
		return shader;
	}
}//namespace