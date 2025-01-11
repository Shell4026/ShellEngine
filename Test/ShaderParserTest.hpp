#pragma once
#include "Core/ExecuteProcess.h"

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

Shader "Default Shader"
{
	Pass "Default Pass"
	{
		Stencil
		{
			Ref 1;
			ReadMask 255;
			WriteMask 255;
			Comp Always;
			Pass Keep;
			Fail Keep;
			ZFail Keep;
		}

		Stage Vertex
		{
			layout(location = 0) in vec3 verts;
			layout(location = 1) in vec2 uvs;
			layout(location = 2) in vec3 normals;

			layout(location = 0) out vec2 fragUvs;
			layout(location = 1) out vec3 fragPos;
			layout(location = 2) out vec3 fragNormals;

			layout(set = 0, binding = 0) uniform MVP
			{
				mat4 model;
				mat4 view;
				mat4 proj;
			} mvp;

			void main()
			{
				gl_Position = mvp.proj * mvp.view * mvp.model * vec4(verts, 1.0f);
				fragUvs = uvs;
				fragPos = (mvp.model * vec4(verts, 1.0f)).xyz;
				fragNormals = normalize((mvp.model * vec4(normals, 0.0f)).xyz);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			layout(location = 0) in vec2 uvs;
			layout(location = 1) in vec3 fragPos;
			layout(location = 2) in vec3 fragNormals;

			layout(set = 0, binding = 1) uniform Lights
			{
				vec4 lightPosRange[10];
				int lightCount;
			} lights;
			layout(set = 1, binding = 0) uniform Material
			{
				float ambient;
			} material;
			layout(set = 1, binding = 1) uniform sampler2D tex;

			void main() 
			{
				float diffuse = 0.f;
				for (int i = 0; i < lights.lightCount; ++i)
				{
					vec3 toLightVec = lights.lightPosRange[i].xyz - fragPos;
					vec3 toLightDir = normalize(toLightVec);
					float lightDis = length(toLightVec);
					float attenuation = clamp(1.0 - (lightDis / lights.lightPosRange[i].w), 0.0, 1.0);
					
					diffuse += max(dot(fragNormals, toLightDir), 0.0) * attenuation;
				}
				outColor = texture(tex, uvs);
				outColor.xyz *= diffuse + material.ambient;
			}
		}
	}
}
)";
	using namespace sh;

	render::ShaderParser parser{};
	auto shaderNode = parser.Parse(shaderCode);

	EXPECT_EQ(shaderNode.shaderName, "Default Shader");
	EXPECT_EQ(shaderNode.version.versionNumber, 430);
	EXPECT_EQ(shaderNode.version.profile, "core");

	auto& passNodes = shaderNode.passes;
	EXPECT_EQ(passNodes.size(), 1);

	auto& passNode = passNodes.front();
	EXPECT_EQ(passNode.name, "Default Pass");
	EXPECT_EQ(passNode.stages.size(), 2);
	EXPECT_EQ(passNode.stencil.state.ref, 1);

	auto& vertexStage = passNode.stages[0];
	EXPECT_EQ(vertexStage.type, render::ShaderAST::StageType::Vertex);
	EXPECT_EQ(vertexStage.in.size(), 3);
	EXPECT_EQ(vertexStage.in[0].binding, 0);
	EXPECT_EQ(vertexStage.in[0].var.type, render::ShaderAST::VariableType::Vec3);
	EXPECT_EQ(vertexStage.in[0].var.size, 1);
	EXPECT_EQ(vertexStage.in[0].var.name, "verts");
	EXPECT_EQ(vertexStage.in[1].var.type, render::ShaderAST::VariableType::Vec2);
	EXPECT_EQ(vertexStage.in[1].var.size, 1);
	EXPECT_EQ(vertexStage.in[1].var.name, "uvs");
	EXPECT_EQ(vertexStage.in[2].var.type, render::ShaderAST::VariableType::Vec3);
	EXPECT_EQ(vertexStage.in[2].var.size, 1);
	EXPECT_EQ(vertexStage.in[2].var.name, "normals");

	EXPECT_EQ(vertexStage.out.size(), 3);
	EXPECT_EQ(vertexStage.out[0].var.type, render::ShaderAST::VariableType::Vec2);
	EXPECT_EQ(vertexStage.out[0].var.size, 1);
	EXPECT_EQ(vertexStage.out[0].var.name, "fragUvs");
	EXPECT_EQ(vertexStage.out[1].var.type, render::ShaderAST::VariableType::Vec3);
	EXPECT_EQ(vertexStage.out[1].var.size, 1);
	EXPECT_EQ(vertexStage.out[1].var.name, "fragPos");
	EXPECT_EQ(vertexStage.out[2].var.type, render::ShaderAST::VariableType::Vec3);
	EXPECT_EQ(vertexStage.out[2].var.size, 1);
	EXPECT_EQ(vertexStage.out[2].var.name, "fragNormals");

	EXPECT_EQ(vertexStage.uniforms.size(), 1);
	EXPECT_EQ(vertexStage.uniforms[0].set, 0);
	EXPECT_EQ(vertexStage.uniforms[0].binding, 0);
	EXPECT_EQ(vertexStage.uniforms[0].bSampler, false);
	EXPECT_EQ(vertexStage.uniforms[0].name, "mvp");
	EXPECT_EQ(vertexStage.uniforms[0].vars.size(), 3);
	EXPECT_EQ(vertexStage.uniforms[0].vars[0].name, "model");
	EXPECT_EQ(vertexStage.uniforms[0].vars[0].type, render::ShaderAST::VariableType::Mat4);
	EXPECT_EQ(vertexStage.uniforms[0].vars[0].size, 1);
	EXPECT_EQ(vertexStage.uniforms[0].vars[1].name, "view");
	EXPECT_EQ(vertexStage.uniforms[0].vars[1].type, render::ShaderAST::VariableType::Mat4);
	EXPECT_EQ(vertexStage.uniforms[0].vars[1].size, 1);
	EXPECT_EQ(vertexStage.uniforms[0].vars[2].name, "proj");
	EXPECT_EQ(vertexStage.uniforms[0].vars[2].type, render::ShaderAST::VariableType::Mat4);
	EXPECT_EQ(vertexStage.uniforms[0].vars[2].size, 1);

	auto& fragmentStage = passNode.stages[1];
	EXPECT_EQ(fragmentStage.type, render::ShaderAST::StageType::Fragment);
	EXPECT_EQ(fragmentStage.in.size(), 3);
	EXPECT_EQ(fragmentStage.in[0].binding, 0);
	EXPECT_EQ(fragmentStage.in[0].var.name, "uvs");
	EXPECT_EQ(fragmentStage.in[0].var.type, render::ShaderAST::VariableType::Vec2);
	EXPECT_EQ(fragmentStage.in[0].var.size, 1);
	EXPECT_EQ(fragmentStage.in[1].binding, 1);
	EXPECT_EQ(fragmentStage.in[1].var.name, "fragPos");
	EXPECT_EQ(fragmentStage.in[1].var.type, render::ShaderAST::VariableType::Vec3);
	EXPECT_EQ(fragmentStage.in[1].var.size, 1);
	EXPECT_EQ(fragmentStage.in[2].binding, 2);
	EXPECT_EQ(fragmentStage.in[2].var.name, "fragNormals");
	EXPECT_EQ(fragmentStage.in[2].var.type, render::ShaderAST::VariableType::Vec3);
	EXPECT_EQ(fragmentStage.in[2].var.size, 1);

	EXPECT_EQ(fragmentStage.out.size(), 1);
	EXPECT_EQ(fragmentStage.out[0].binding, 0);
	EXPECT_EQ(fragmentStage.out[0].var.name, "outColor");
	EXPECT_EQ(fragmentStage.out[0].var.type, render::ShaderAST::VariableType::Vec4);
	EXPECT_EQ(fragmentStage.out[0].var.size, 1);

	EXPECT_EQ(fragmentStage.uniforms.size(), 3);
	EXPECT_EQ(fragmentStage.uniforms[0].bSampler, false);
	EXPECT_EQ(fragmentStage.uniforms[0].set, 0);
	EXPECT_EQ(fragmentStage.uniforms[0].binding, 1);
	EXPECT_EQ(fragmentStage.uniforms[0].name, "lights");
	EXPECT_EQ(fragmentStage.uniforms[0].vars.size(), 2);
	EXPECT_EQ(fragmentStage.uniforms[0].vars[0].name, "lightPosRange");
	EXPECT_EQ(fragmentStage.uniforms[0].vars[0].type, render::ShaderAST::VariableType::Vec4);
	EXPECT_EQ(fragmentStage.uniforms[0].vars[0].size, 10);
	EXPECT_EQ(fragmentStage.uniforms[0].vars[1].name, "lightCount");
	EXPECT_EQ(fragmentStage.uniforms[0].vars[1].type, render::ShaderAST::VariableType::Int);
	EXPECT_EQ(fragmentStage.uniforms[0].vars[1].size, 1);
	EXPECT_EQ(fragmentStage.uniforms[1].bSampler, false);
	EXPECT_EQ(fragmentStage.uniforms[1].set, 1);
	EXPECT_EQ(fragmentStage.uniforms[1].binding, 0);
	EXPECT_EQ(fragmentStage.uniforms[1].name, "material");
	EXPECT_EQ(fragmentStage.uniforms[1].vars.size(), 1);
	EXPECT_EQ(fragmentStage.uniforms[1].vars[0].name, "ambient");
	EXPECT_EQ(fragmentStage.uniforms[1].vars[0].type, render::ShaderAST::VariableType::Float);
	EXPECT_EQ(fragmentStage.uniforms[1].vars[0].size, 1);
	EXPECT_EQ(fragmentStage.uniforms[2].bSampler, true);
	EXPECT_EQ(fragmentStage.uniforms[2].set, 1);
	EXPECT_EQ(fragmentStage.uniforms[2].binding, 1);
	EXPECT_EQ(fragmentStage.uniforms[2].name, "tex");
	EXPECT_EQ(fragmentStage.uniforms[2].vars.size(), 0);
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