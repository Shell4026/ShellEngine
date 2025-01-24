#pragma once
#include "Core/ExecuteProcess.h"

#include "Render/ShaderLexer.h"
#include "Render/ShaderParser.h"
#include "Render/ShaderGenerator.h"
#include "Render/VulkanImpl/VulkanShaderPassBuilder.h"

#include "Game/ShaderLoader.h"

#ifdef Bool
#undef Bool
#endif

#include <gtest/gtest.h>

TEST(ShaderParserTest, ParserTest)
{
	const char* shaderCode = R"(
#version 430 core

Shader "Outline Shader"
{
	Property
	{
		float ambient;
		float outlineWidth;
		vec4 outlineColor;
		sampler2D tex;
	}

	Pass "Default Pass"
	{
		LightingPass "Forward"

		Stencil // 항상 버퍼를 1로 쓴다.
		{
			Ref 1;
			ReadMask 255;
			WriteMask 255;
			Comp Always;
			Pass Replace;
			Fail Keep;
			ZFail Keep;
		}
		Stage Vertex
		{
			layout(location = 0) out vec2 fragUvs;
			layout(location = 1) out vec3 fragPos;
			layout(location = 2) out vec3 fragNormals;

			void main()
			{
				gl_Position = MVP.proj * MVP.view * MVP.model * vec4(VERTEX, 1.0f);
				fragUvs = UV;
				fragPos = (MVP.model * vec4(VERTEX, 1.0f)).xyz;
				fragNormals = normalize((MVP.model * vec4(NORMAL, 0.0f)).xyz);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			layout(location = 0) in vec2 uvs;
			layout(location = 1) in vec3 fragPos;
			layout(location = 2) in vec3 fragNormals;

			uniform float ambient;
			uniform sampler2D tex;

			void RedColor(out vec4 color)
			{
				color = vec4(1.f, 0.f, 0.f, 1.0f);
			}

			void main() 
			{
				float diffuse = 0.f;
				for (int i = 0; i < LIGHT.count; ++i)
				{
					vec3 toLightVec = LIGHT.pos[i].xyz - fragPos;
					vec3 toLightDir = normalize(toLightVec);
					float lightDis = length(toLightVec);
					float attenuation = clamp(1.0 - (lightDis / LIGHT.range[i]), 0.0, 1.0);
					
					diffuse += max(dot(fragNormals, toLightDir), 0.0) * attenuation;
				}
				outColor = texture(tex, uvs);
				outColor.xyz *= diffuse + ambient;
			}
		}
	}
	Pass "Outline Pass"
	{
		LightingPass "Forward"

		Stencil // 1이 아닌 곳에 그린다.
		{
			Ref 1;
			Comp NotEqual;
		}

		Stage Vertex
		{
			uniform float outlineWidth;
			
			void main()
			{
				vec3 vert = VERTEX;
				vert.xyz *= NORMAL * outlineWidth;
				gl_Position = MVP.proj * MVP.view * MVP.model * vec4(VERTEX, 1.0f);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			uniform vec4 outlineColor;
			const float offset = 0.5f;

			void main() 
			{
				outColor = outlineColor * offset;
			}
		}
	}
}
)";
	using namespace sh;

	render::ShaderLexer lexer{};
	render::ShaderParser parser{};
	render::ShaderAST::ShaderNode shaderNode{};
	auto tokens = lexer.Lex(shaderCode);
	shaderNode = parser.Parse(tokens);

	render::ShaderGenerator::GenerateShaderFile(shaderNode.shaderName, shaderNode.passes[0], std::filesystem::current_path());

	EXPECT_EQ(shaderNode.shaderName, "Outline Shader");
	EXPECT_EQ(shaderNode.version.versionNumber, 430);
	EXPECT_EQ(shaderNode.version.profile, "core");

	auto& passNodes = shaderNode.passes;
	EXPECT_EQ(passNodes.size(), 2);

	EXPECT_EQ(shaderNode.properties[0].name, "ambient");
	EXPECT_EQ(shaderNode.properties[1].name, "outlineWidth");
	EXPECT_EQ(shaderNode.properties[2].name, "outlineColor");
	EXPECT_EQ(shaderNode.properties[3].name, "tex");
	EXPECT_EQ(shaderNode.properties[3].type, render::ShaderAST::VariableType::Sampler);

	EXPECT_EQ(passNodes[0].name, "Default Pass");
	EXPECT_EQ(passNodes[0].lightingPass, "Forward");
	EXPECT_EQ(passNodes[0].stencil.ref, 1);
	EXPECT_EQ(passNodes[1].name, "Outline Pass");
	EXPECT_EQ(passNodes[1].lightingPass, "Forward");
	EXPECT_EQ(passNodes[1].stencil.compareOp, render::StencilState::CompareOp::NotEqual);
}

TEST(ShaderParserTest, ShaderCompileTest)
{
	using namespace sh;
	{
		std::vector<std::string> args{ "Default_Shader_Default_Pass.vert", "-o", "testVert.spv" };
		std::string output;
#if _WIN32
		EXPECT_TRUE(core::ExecuteProcess::Execute("glslc.exe", args, output));
#elif __linux__
		EXPECT_TRUE(core::ExecuteProcess::Execute("glslc", args, output));
		std::cout << output << '\n';
#endif
	}
	{
		std::vector<std::string> args{ "Default_Shader_Default_Pass.frag", "-o", "testFrag.spv" };
		std::string output;
#if _WIN32
		EXPECT_TRUE(core::ExecuteProcess::Execute("glslc.exe", args, output));
#elif __linux__
		EXPECT_TRUE(core::ExecuteProcess::Execute("glslc", args, output));
#endif
		std::cout << output << '\n';
	}
}