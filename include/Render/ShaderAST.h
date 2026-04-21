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
			IVec4,
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
			uint32_t arraySize = 1; // 배열 원소 개수 (단일 변수면 1)
			std::string name;
			std::string defaultValue;
			VariableAttribute attribute = VariableAttribute::None;
			bool bDynamicArray = false; // SSBO 끝의 가변 배열 (arraySize 무시)

			VariableNode() = default;
			VariableNode(VariableType type, uint32_t arraySize, std::string name) :
				type(type), arraySize(arraySize), name(std::move(name))
			{
			}
			static auto MakeDynamicArray(VariableType type, std::string name) -> VariableNode
			{
				VariableNode v;
				v.type = type;
				v.bDynamicArray = true;
				v.name = std::move(name);
				return v;
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
		enum class BufferType : uint8_t
		{
			Uniform, // UBO (std140)
			Storage, // SSBO (std430)
			Sampler,
			PushConstant // 푸시 상수
		};
		struct BufferNode : core::ISerializable
		{
			BufferType bufferType = BufferType::Uniform;
			uint32_t set = 0;
			uint32_t binding = 0;
			std::string name;
			std::vector<VariableNode> vars;

			SH_RENDER_API auto Serialize() const -> core::Json override;
			SH_RENDER_API void Deserialize(const core::Json& json) override;
		};
		struct StageNode : core::ISerializable
		{
			StageType type;
			std::vector<LayoutNode> in;
			std::vector<LayoutNode> out;
			std::vector<BufferNode> buffers;
			std::vector<std::string> declaration;
			std::vector<std::string> functions;
			std::string code;
			int lightingBinding = -1;
			int skinBinding = -1;

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
			bool bZTest = true;

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