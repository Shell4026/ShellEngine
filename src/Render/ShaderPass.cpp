#include "ShaderPass.h"

namespace sh::render
{
	ShaderPass::ShaderPass(const ShaderAST::PassNode& passNode, ShaderType type) :
		type(type), lightingPassName(passNode.lightingPass)
	{
		SetStencilState(passNode.stencil);
		cull = passNode.cullMode;
		bZWrite = passNode.zwrite;
		bZTest = passNode.bZTest;
		colorMask = passNode.colorMask;
		FillAttributes(passNode);
		for (const ShaderAST::StageNode& stage : passNode.stages)
		{
			if (stage.lightingBinding != -1)
				lightingBinding = stage.lightingBinding;
			if (stage.skinBinding != -1)
				skinBinding = stage.skinBinding;
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
		lightingBinding(other.lightingBinding),
		skinBinding(other.skinBinding)
	{
	}
	ShaderPass::~ShaderPass() = default;

	auto ShaderPass::operator=(ShaderPass&& other) noexcept -> ShaderPass&
	{
		if (&other == this)
			return *this;

		type = other.type;
		lightingPassName = other.lightingPassName;
		attrs = std::move(other.attrs);
		attridx = std::move(other.attridx);
		vertexUniforms = std::move(other.vertexUniforms);
		fragmentUniforms = std::move(other.fragmentUniforms);
		samplerUniforms = std::move(other.samplerUniforms);
		constantNameMap = std::move(other.constantNameMap);
		cull = other.cull;
		colorMask = other.colorMask;
		bZWrite = other.bZWrite;
		bHasConstant = other.bHasConstant;
		lightingBinding = other.lightingBinding;
		skinBinding = other.skinBinding;

		return *this;
	}

	SH_RENDER_API auto ShaderPass::HasUniformMember(const std::string& name, ShaderStage stage) const -> const UniformStructLayout*
	{
		if (stage == ShaderStage::Vertex)
		{
			for (const UniformStructLayout& uniformStruct : GetVertexUniforms())
			{
				if (uniformStruct.HasMember(name))
					return &uniformStruct;
			}
		}
		else if (stage == ShaderStage::Fragment)
		{
			for (const UniformStructLayout& uniformStruct : GetFragmentUniforms())
			{
				if (uniformStruct.HasMember(name))
					return &uniformStruct;
			}
			for (const UniformStructLayout& uniformStruct : GetSamplerUniforms())
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
			for (const ShaderAST::BufferNode& bufferNode : stage.buffers)
			{
				UniformStructLayout::Usage uniformUsage = static_cast<UniformStructLayout::Usage>(bufferNode.set);

				UniformStructLayout::Kind kind = UniformStructLayout::Kind::Uniform;
				switch (bufferNode.bufferType)
				{
				case render::ShaderAST::BufferType::Storage: kind = UniformStructLayout::Kind::Storage; break;
				case render::ShaderAST::BufferType::Sampler: kind = UniformStructLayout::Kind::Sampler; break;
				case render::ShaderAST::BufferType::PushConstant: kind = UniformStructLayout::Kind::PushConstant; break;
				case render::ShaderAST::BufferType::Uniform: kind = UniformStructLayout::Kind::Uniform; break;
				}

				render::UniformStructLayout uniformLayout{ bufferNode.name, bufferNode.binding, uniformUsage, stageType, kind };

				bHasConstant |= (kind == UniformStructLayout::Kind::PushConstant);

				if (kind != UniformStructLayout::Kind::Sampler)
				{
					for (auto& var : bufferNode.vars)
					{
						auto addFn = [&](auto typeTag)
						{
							using T = decltype(typeTag);
							if (var.bDynamicArray)
								uniformLayout.AddDynamicArrayMember<T>(var.name);
							else
								uniformLayout.AddArrayMember<T>(var.name, var.arraySize);
						};
						switch (var.type)
						{
						case render::ShaderAST::VariableType::Vec4: addFn(glm::vec4{}); break;
						case render::ShaderAST::VariableType::Vec3: addFn(glm::vec3{}); break;
						case render::ShaderAST::VariableType::Vec2: addFn(glm::vec2{}); break;
						case render::ShaderAST::VariableType::Mat4: addFn(glm::mat4{}); break;
						case render::ShaderAST::VariableType::Mat3: addFn(glm::mat3{}); break;
						case render::ShaderAST::VariableType::Float: addFn(float{}); break;
						case render::ShaderAST::VariableType::Int:  addFn(int{}); break;
						}
					}
				}
				else
					uniformLayout.AddMember<render::Texture>(bufferNode.name);

				AddUniformLayout(stageType, std::move(uniformLayout));
			}
		}
	}
	auto ShaderPass::IsSamplerLayout(const UniformStructLayout& layout) const -> bool
	{
		return layout.IsSampler();
	}
}//namespace