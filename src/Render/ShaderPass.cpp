#include "ShaderPass.h"

namespace sh::render
{
	void ShaderPass::FillAttributes(const render::ShaderAST::PassNode& passNode)
	{
		for (auto& stage : passNode.stages)
		{
			auto stageType = (stage.type == render::ShaderAST::StageType::Vertex) ?
				render::ShaderStage::Vertex :
				render::ShaderStage::Fragment;

			if (stage.type == render::ShaderAST::StageType::Vertex)
			{
				for (auto& in : stage.in)
				{
					switch (in.var.type)
					{
					case render::ShaderAST::VariableType::Vec4:  AddAttribute<glm::vec4>(in.var.name, in.binding); break;
					case render::ShaderAST::VariableType::Vec3:  AddAttribute<glm::vec3>(in.var.name, in.binding); break;
					case render::ShaderAST::VariableType::Vec2:  AddAttribute<glm::vec2>(in.var.name, in.binding); break;
					case render::ShaderAST::VariableType::Mat4:  AddAttribute<glm::mat4>(in.var.name, in.binding); break;
					case render::ShaderAST::VariableType::Mat3:  AddAttribute<glm::mat3>(in.var.name, in.binding); break;
					case render::ShaderAST::VariableType::Float: AddAttribute<    float>(in.var.name, in.binding); break;
					case render::ShaderAST::VariableType::Int:   AddAttribute<      int>(in.var.name, in.binding); break;
					}
				}
			}
			for (auto& uniform : stage.uniforms)
			{
				UniformStructLayout::Type uniformType = static_cast<UniformStructLayout::Type>(uniform.set);

				render::UniformStructLayout uniformLayout{ uniform.name, uniform.binding, uniformType, stageType, uniform.bConstant };

				bHasConstant |= uniform.bConstant;

				if (!uniform.bSampler)
				{
					for (auto& var : uniform.vars)
					{
						switch (var.type)
						{
						case render::ShaderAST::VariableType::Vec4:  uniformLayout.AddArrayMember<glm::vec4>(var.name, var.size); break;
						case render::ShaderAST::VariableType::Vec3:  uniformLayout.AddArrayMember<glm::vec3>(var.name, var.size); break;
						case render::ShaderAST::VariableType::Vec2:  uniformLayout.AddArrayMember<glm::vec2>(var.name, var.size); break;
						case render::ShaderAST::VariableType::Mat4:  uniformLayout.AddArrayMember<glm::mat4>(var.name, var.size); break;
						case render::ShaderAST::VariableType::Mat3:  uniformLayout.AddArrayMember<glm::mat3>(var.name, var.size); break;
						case render::ShaderAST::VariableType::Float: uniformLayout.AddArrayMember<    float>(var.name, var.size); break;
						case render::ShaderAST::VariableType::Int:   uniformLayout.AddArrayMember<      int>(var.name, var.size); break;
						}
					}
				}
				else
					uniformLayout.AddMember<render::Texture>(uniform.name);

				AddUniformLayout(stageType, std::move(uniformLayout));
			}
		}
	}
	auto ShaderPass::IsSamplerLayout(const UniformStructLayout& layout) const -> bool
	{
		for (auto& member : layout.GetMembers())
		{
			if (member.isSampler)
				return true;
		}
		return false;
	}

	ShaderPass::ShaderPass(const ShaderAST::PassNode& passNode, ShaderType type) :
		type(type), lightingPassName(passNode.lightingPass)
	{
		SetStencilState(passNode.stencil);
		cull = passNode.cullMode;
		bZWrite = passNode.zwrite;
		colorMask = passNode.colorMask;
		FillAttributes(passNode);
		for (auto& stage : passNode.stages)
		{
			if (stage.bUseLighting)
			{
				bUseLighting = true;
				break;
			}
		}
		if (!passNode.constants.empty())
		{
			std::size_t cursor = 0;

			for (int i = 0; i < passNode.constants.size(); ++i)
			{
				const auto& varNode = passNode.constants[i];

				uint32_t offset = cursor;
				uint32_t size = 0;
				switch (varNode.type)
				{
				case ShaderAST::VariableType::Boolean: [[fallthrough]];
				case ShaderAST::VariableType::Int: [[fallthrough]];
				case ShaderAST::VariableType::Float:
					size = 4;
					break;
				}
				constantNameMap[varNode.name] = 
				{
					offset,
					size,
					static_cast<uint32_t>(i)
				};

				cursor += size;
			}
			constantSize = cursor;
		}
	}

	ShaderPass::ShaderPass(ShaderPass&& other) noexcept :
		type(other.type), lightingPassName(other.lightingPassName),
		attrs(std::move(other.attrs)), attridx(std::move(other.attridx)),
		vertexUniforms(std::move(other.vertexUniforms)),
		fragmentUniforms(std::move(other.fragmentUniforms)),
		samplerUniforms(std::move(other.samplerUniforms)),
		constantNameMap(std::move(other.constantNameMap)),
		cull(other.cull), colorMask(other.colorMask),
		bZWrite(other.bZWrite),
		bHasConstant(other.bHasConstant),
		bUseLighting(other.bUseLighting)
	{
	}
	ShaderPass::~ShaderPass()
	{
	}
	auto ShaderPass::GetShaderType() const -> ShaderType
	{
		return type;
	}

	void ShaderPass::operator=(ShaderPass&& other) noexcept
	{
		type = other.type;
		lightingPassName = other.lightingPassName;
		attrs = std::move(other.attrs);
		attridx = std::move(other.attridx);
		vertexUniforms = std::move(other.vertexUniforms);
		fragmentUniforms = std::move(other.fragmentUniforms);
		samplerUniforms = std::move(other.samplerUniforms);
		constantNameMap = std::move(other.constantNameMap);
	}

	void ShaderPass::AddUniformLayout(ShaderStage stage, const UniformStructLayout& layout)
	{
		if (stage == ShaderStage::Vertex)
			vertexUniforms.push_back(layout);
		else if (stage == ShaderStage::Fragment)
		{
			if (IsSamplerLayout(layout) == false)
				fragmentUniforms.push_back(layout);
			else
				samplerUniforms.push_back(layout);
		}
	}
	void ShaderPass::AddUniformLayout(ShaderStage stage, UniformStructLayout&& layout)
	{
		if (stage == ShaderStage::Vertex)
			vertexUniforms.push_back(std::move(layout));
		else if (stage == ShaderStage::Fragment)
		{
			if (IsSamplerLayout(layout) == false)
				fragmentUniforms.push_back(std::move(layout));
			else
				samplerUniforms.push_back(std::move(layout));
		}
	}
	SH_RENDER_API auto ShaderPass::HasUniformMember(const std::string& name, ShaderStage stage) const -> const UniformStructLayout*
	{
		if (stage == ShaderStage::Vertex)
		{
			for (auto& uniformStruct : GetVertexUniforms())
			{
				if (uniformStruct.HasMember(name))
					return &uniformStruct;
			}
		}
		else if (stage == ShaderStage::Fragment)
		{
			for (auto& uniformStruct : GetFragmentUniforms())
			{
				if (uniformStruct.HasMember(name))
					return &uniformStruct;
			}
			for (auto& uniformStruct : GetSamplerUniforms())
			{
				if (uniformStruct.HasMember(name))
					return &uniformStruct;
			}
		}
		return nullptr;
	}

	void ShaderPass::SetStencilState(StencilState stencilState)
	{
		this->stencilState = stencilState;
	}

	SH_RENDER_API auto ShaderPass::GetStencilState() const -> const StencilState&
	{
		return stencilState;
	}
	SH_RENDER_API auto ShaderPass::GetCullMode() const -> CullMode
	{
		return cull;
	}
	SH_RENDER_API auto ShaderPass::GetZWrite() const -> bool
	{
		return bZWrite;
	}
	SH_RENDER_API auto ShaderPass::GetColorMask() const -> uint8_t
	{
		return colorMask;
	}
	SH_RENDER_API auto ShaderPass::GetLightingPassName() const -> const core::Name&
	{
		return lightingPassName;
	}

	bool ShaderPass::HasAttribute(const std::string& name) const
	{
		auto it = attridx.find(name);
		if (it == attridx.end())
			return false;
		return true;
	}

	auto ShaderPass::GetAttribute(const std::string& name) const -> std::optional<AttributeData>
	{
		auto it = attridx.find(name);
		if (it == attridx.end())
			return {};
		
		return attrs[it->second];
	}
	auto ShaderPass::GetAttributes() const -> const std::vector<AttributeData>&
	{
		return attrs;
	}
	auto ShaderPass::GetVertexUniforms() const -> const std::vector<UniformStructLayout>&
	{
		return vertexUniforms;
	}
	auto ShaderPass::GetFragmentUniforms() const -> const std::vector<UniformStructLayout>&
	{
		return fragmentUniforms;
	}
	auto ShaderPass::GetSamplerUniforms() const -> const std::vector<UniformStructLayout>&
	{
		return samplerUniforms;
	}
	SH_RENDER_API auto ShaderPass::HasConstantUniform() const -> bool
	{
		return bHasConstant;
	}
	SH_RENDER_API auto ShaderPass::IsUsingLight() const -> bool
	{
		return bUseLighting;
	}
	SH_RENDER_API auto ShaderPass::GetConstantsInfo(const std::string& name) const -> const ConstantInfo*
	{
		auto it = constantNameMap.find(name);
		if (it == constantNameMap.end())
			return nullptr;
		return &it->second;
	}
	SH_RENDER_API auto ShaderPass::GetConstantSize() const -> std::size_t
	{
		return constantSize;
	}
	SH_RENDER_API void ShaderPass::StoreShaderCode(ShaderCode&& shaderCode)
	{
		this->shaderCode = std::move(shaderCode);
	}
	SH_RENDER_API auto ShaderPass::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();

		mainJson["shaderPass"] = core::Json{};
		core::Json& shaderPassJson = mainJson["shaderPass"];

		shaderPassJson["vertShaderData"] = shaderCode.vert;
		shaderPassJson["fragShaderData"] = shaderCode.frag;

		return mainJson;
	}
	SH_RENDER_API void ShaderPass::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);

		if (!json.contains("shaderPass"))
			return;

		const auto& shaderPassJson = json["shaderPass"];
		if (shaderPassJson.contains("vertShaderData"))
			shaderCode.vert = shaderPassJson["vertShaderData"].get<std::vector<uint8_t>>();
		if (shaderPassJson.contains("fragShaderData"))
			shaderCode.frag = shaderPassJson["fragShaderData"].get<std::vector<uint8_t>>();
	}
}