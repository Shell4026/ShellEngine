#pragma once
#include "StencilState.h"
#include "ShaderEnum.h"

#include <string>
#include <vector>
#include <memory>

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
			Mat2,
			Vec4,
			Vec3,
			Vec2,
			Float,
			Int,
			Sampler
		};
		enum class VariableAttribute
		{
			None,
			Local
		};

		struct VariableNode
		{
			VariableType type;
			int size = 1;
			std::string name;
			VariableAttribute attribute;
		};
		struct LayoutNode
		{
			uint32_t binding;
			VariableNode var;
		};
		struct UBONode
		{
			uint32_t set;
			uint32_t binding;
			std::string name;
			bool bSampler = false;
			bool bConstant = false;
			std::vector<VariableNode> vars;
		};
		struct StageNode
		{
			StageType type;
			std::vector<LayoutNode> in;
			std::vector<LayoutNode> out;
			std::vector<UBONode> uniforms;
			std::vector<std::string> declaration;
			std::vector<std::string> functions;
			std::string code;
		};

		struct PassNode
		{
			std::string name;
			std::string lightingPass;
			StencilState stencil;
			CullMode cullMode;
			uint8_t colorMask = 15;
			std::vector<StageNode> stages;
			bool zwrite = true;
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
			std::vector<VariableNode> properties;
			std::vector<PassNode> passes;
		};
	};
}//namespace