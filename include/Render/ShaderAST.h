#pragma once
#include "Export.h"
#include "StencilState.h"
#include "ShaderEnum.h"

#include "Core/ISerializable.h"

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
			Sampler,
			Boolean
		};
		enum class VariableAttribute
		{
			None,
			Local
		};

		struct VariableNode : core::ISerializable
		{
			VariableType type = VariableType::Int;
			int size = 1;
			std::string name;
			std::string defaultValue;
			VariableAttribute attribute = VariableAttribute::None;

			VariableNode() = default;
			VariableNode(VariableType type, int size, std::string name) :
				type(type), size(size), name(name)
			{
			}

			SH_RENDER_API auto Serialize() const -> core::Json override;
			SH_RENDER_API void Deserialize(const core::Json& json) override;
		};
		struct LayoutNode : core::ISerializable
		{
			uint32_t binding;
			VariableNode var;

			LayoutNode() = default;
			LayoutNode(uint32_t binding) :
				binding(binding)
			{
			}
			LayoutNode(uint32_t binding, const VariableNode& var) :
				binding(binding), var(var)
			{
			}
			LayoutNode(uint32_t binding, VariableNode&& var) :
				binding(binding), var(std::move(var))
			{
			}

			SH_RENDER_API auto Serialize() const -> core::Json override;
			SH_RENDER_API void Deserialize(const core::Json& json) override;
		};
		struct UBONode : core::ISerializable
		{
			uint32_t set;
			uint32_t binding;
			std::string name;
			bool bSampler = false;
			bool bConstant = false;
			std::vector<VariableNode> vars;

			SH_RENDER_API auto Serialize() const -> core::Json override;
			SH_RENDER_API void Deserialize(const core::Json& json) override;
		};
		struct StageNode : core::ISerializable
		{
			StageType type;
			std::vector<LayoutNode> in;
			std::vector<LayoutNode> out;
			std::vector<UBONode> uniforms;
			std::vector<std::string> declaration;
			std::vector<std::string> functions;
			std::string code;
			bool bUseLighting = false;

			SH_RENDER_API auto Serialize() const -> core::Json override;
			SH_RENDER_API void Deserialize(const core::Json& json) override;
		};

		struct PassNode : core::ISerializable
		{
			std::string name;
			std::string lightingPass;
			StencilState stencil;
			CullMode cullMode;
			uint8_t colorMask = 15;
			std::vector<VariableNode> constants;
			std::vector<StageNode> stages;
			bool zwrite = true;

			SH_RENDER_API auto Serialize() const -> core::Json override;
			SH_RENDER_API void Deserialize(const core::Json& json) override;
		};

		struct VersionNode : core::ISerializable
		{
			int versionNumber;
			std::string profile; // core, compatibility, ...

			SH_RENDER_API auto Serialize() const -> core::Json override;
			SH_RENDER_API void Deserialize(const core::Json& json) override;
		};

		struct ShaderNode : core::ISerializable
		{
			VersionNode version;
			std::string shaderName;
			std::vector<VariableNode> properties;
			std::vector<PassNode> passes;

			SH_RENDER_API auto Serialize() const -> core::Json override;
			SH_RENDER_API void Deserialize(const core::Json& json) override;
		};
	};
}//namespace