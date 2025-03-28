
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

	ShaderPass::ShaderPass(const ShaderAST::PassNode& pass, ShaderType type) :
		type(type), lightingPassName(pass.lightingPass)
	{
		SetStencilState(pass.stencil);
		cull = pass.cullMode;
		bZWrite = pass.zwrite;
		colorMask = pass.colorMask;
		FillAttributes(pass);
	}

	ShaderPass::ShaderPass(ShaderPass&& other) noexcept :
		type(other.type), lightingPassName(other.lightingPassName),
		attrs(std::move(other.attrs)), attridx(std::move(other.attridx)),
		vertexUniforms(std::move(other.vertexUniforms)),
		fragmentUniforms(std::move(other.fragmentUniforms)),
		samplerUniforms(std::move(other.samplerUniforms))
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
}