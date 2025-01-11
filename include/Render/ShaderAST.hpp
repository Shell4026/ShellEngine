#pragma once
#include "StencilState.h"

#include <string>
#include <vector>

namespace sh::render
{
	struct ShaderAST
	{
		enum class StageType
		{
			Vertex,
			Fragment,
			Unknown
		};
		enum class VariableType
		{
			Mat4,
			Mat3,
			Vec4,
			Vec3,
			Vec2,
			Float,
			Int
		};

		struct VariableNode
		{
			VariableType type;
			int size = 1;
			std::string name;
		};
		struct LayoutNode
		{
			uint32_t binding;
			VariableNode var;
		};
		struct UBONode
		{
			int set;
			int binding;
			std::string name;
			bool bSampler = false;
			std::vector<VariableNode> vars;
		};
		struct StencilNode
		{
			StencilState state;
		};

		struct StageNode
		{
			StageType type;
			std::vector<LayoutNode> in;
			std::vector<LayoutNode> out;
			std::vector<UBONode> uniforms;
			std::string code;
		};

		struct PassNode
		{
			std::string name;
			StencilNode stencil;
			std::vector<StageNode> stages;
		};

		struct VersionNode
		{
			int versionNumber;
			std::string profile; // core, compatibility, ...
		};

		struct ShaderNode
		{
			VersionNode version;
			std::string shaderName;
			std::vector<PassNode> passes;
		};

	};
}//namespace