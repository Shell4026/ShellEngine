#include "ShaderAST.h"

namespace sh::render
{
	SH_RENDER_API auto ShaderAST::VariableNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["type"] = static_cast<int>(type);
		json["structType"] = structType;
		json["arraySize"] = arraySize;
		json["name"] = name;
		json["default"] = defaultValue;
		json["attribute"] = static_cast<int>(attribute);
		json["dynamicArray"] = bDynamicArray;
		return json;
	}
	SH_RENDER_API void ShaderAST::VariableNode::Deserialize(const core::Json& json)
	{
		type = static_cast<VariableType>(json.at("type").get<int>());
		if (json.contains("structType"))
			structType = json["structType"].get_ref<const std::string&>();
		if (json.contains("arraySize"))
		{
			arraySize = json.at("arraySize").get<uint32_t>();
			bDynamicArray = json.value("dynamicArray", false);
		}
		name = json.at("name").get<std::string>();
		if (json.contains("default"))
			defaultValue = json["default"].get<std::string>();
		attribute = static_cast<VariableAttribute>(json.at("attribute").get<int>());
	}
	SH_RENDER_API auto ShaderAST::VariableNode::MakeDynamicArray(StructNode& structNode, std::string name) -> VariableNode
	{
		VariableNode v;
		v.type = VariableType::Struct;
		v.structType = structNode.name;
		v.bDynamicArray = true;
		v.name = std::move(name);
		return v;
	}

	SH_RENDER_API auto ShaderAST::StructNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["name"] = name;
		core::Json& varsJson = json["vars"];
		for (const VariableNode& varNode : vars)
			varsJson.push_back(varNode.Serialize());
		return json;
	}
	SH_RENDER_API void ShaderAST::StructNode::Deserialize(const core::Json& json)
	{
		if (json.contains("name"))
			name = json["name"].get_ref<const std::string&>();
		if (json.contains("vars"))
		{
			const core::Json& varsJson = json["vars"];
			vars.reserve(varsJson.size());
			for (const core::Json& varJson : varsJson)
			{
				VariableNode varNode;
				varNode.Deserialize(varJson);
				vars.push_back(std::move(varNode));
			}
		}
	}
	SH_RENDER_API auto ShaderAST::LayoutNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["binding"] = binding;
		json["var"] = var.Serialize();
		return json;
	}

	SH_RENDER_API void ShaderAST::LayoutNode::Deserialize(const core::Json& json)
	{
		binding = json.at("binding").get<uint32_t>();
		var.Deserialize(json.at("var"));
	}
	SH_RENDER_API auto ShaderAST::BufferNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["bufferType"] = static_cast<int>(bufferType);
		json["access"] = static_cast<int>(access);
		json["anonymous"] = bAnonymousInstance;
		json["set"] = set;
		json["binding"] = binding;
		json["name"] = name;
		core::Json varArray = core::Json::array();
		for (const auto& v : vars)
		{
			varArray.push_back(v.Serialize());
		}
		json["vars"] = std::move(varArray);
		return json;
	}
	SH_RENDER_API void ShaderAST::BufferNode::Deserialize(const core::Json& json)
	{
		set = json.at("set").get<uint32_t>();
		binding = json.at("binding").get<uint32_t>();
		name = json.at("name").get<std::string>();

		if (json.contains("bufferType"))
			bufferType = static_cast<BufferType>(json.at("bufferType").get<int>());
		if (json.contains("access"))
			access = static_cast<BufferAccess>(json.at("access").get<int>());
		if (json.contains("anonymous"))
			bAnonymousInstance = json.at("anonymous").get<bool>();

		vars.clear();
		for (const auto& v : json.at("vars"))
		{
			VariableNode var;
			var.Deserialize(v);
			vars.push_back(std::move(var));
		}
	}
	SH_RENDER_API auto ShaderAST::StageNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["type"] = static_cast<int>(type);
		json["lightingBinding"] = lightingBinding;
		json["skinBinding"] = skinBinding;
		json["code"] = code;

		// in
		core::Json inArray = core::Json::array();
		for (const auto& i : in)
			inArray.push_back(i.Serialize());
		json["in"] = std::move(inArray);

		// out
		core::Json outArray = core::Json::array();
		for (const auto& o : out)
			outArray.push_back(o.Serialize());
		json["out"] = std::move(outArray);

		// structs
		core::Json& structsJson = json["structs"];
		for (const StructNode& structNode : structs)
			structsJson.push_back(structNode.Serialize());

		// uniforms
		core::Json bufferArray = core::Json::array();
		for (const auto& u : buffers)
			bufferArray.push_back(u.Serialize());
		json["buffers"] = std::move(bufferArray);

		// declaration
		json["declaration"] = declaration;

		// functions
		json["functions"] = functions;

		return json;
	}

	SH_RENDER_API void ShaderAST::StageNode::Deserialize(const core::Json& json)
	{
		type = static_cast<StageType>(json.at("type").get<int>());
		lightingBinding = json.value("lightingBinding", -1);
		skinBinding = json.value("skinBinding", -1);
		code = json.at("code").get<std::string>();

		// in
		in.clear();
		out.clear();
		if (json.contains("in"))
		{
			for (const auto& i : json.at("in"))
			{
				LayoutNode l;
				l.Deserialize(i);
				in.push_back(std::move(l));
			}
		}

		// out
		out.clear();
		if (json.contains("out"))
		{
			for (const auto& o : json.at("out"))
			{
				LayoutNode l;
				l.Deserialize(o);
				out.push_back(std::move(l));
			}
		}

		// structs
		structs.clear();
		if (json.contains("structs"))
		{
			for (const core::Json& structJson : json.at("structs"))
			{
				StructNode structNode;
				structNode.Deserialize(structJson);
				structs.push_back(std::move(structNode));
			}
		}

		// uniforms
		buffers.clear();
		if (json.contains("buffers"))
		{
			for (const auto& u : json.at("buffers"))
			{
				BufferNode ubo;
				ubo.Deserialize(u);
				buffers.push_back(std::move(ubo));
			}
		}

		// declaration
		if (json.contains("declaration"))
			declaration = json.at("declaration").get<std::vector<std::string>>();

		// functions
		if (json.contains("functions"))
			functions = json.at("functions").get<std::vector<std::string>>();
	}
	SH_RENDER_API auto ShaderAST::StageNode::GetStructNode(const std::string& structName) const -> const StructNode*
	{
		for (const StructNode& structNode : structs)
		{
			if (structNode.name == structName)
				return &structNode;
		}
		return nullptr;
	}
	SH_RENDER_API auto ShaderAST::PassNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["name"] = name;
		json["lightingPass"] = lightingPass;
		json["cullMode"] = static_cast<int>(cullMode);
		json["colorMask"] = colorMask;
		json["zwrite"] = zwrite;
		json["ztest"] = bZTest;

		json["stencil"] = stencil.Serialize();

		// constants
		core::Json constantArray = core::Json::array();
		for (const auto& u : constants)
			constantArray.push_back(u.Serialize());
		json["constants"] = std::move(constantArray);

		// stages
		core::Json stagesArray = core::Json::array();
		for (const auto& s : stages)
			stagesArray.push_back(s.Serialize());
		json["stages"] = std::move(stagesArray);

		return json;
	}
	SH_RENDER_API void ShaderAST::PassNode::Deserialize(const core::Json& json)
	{
		name = json.at("name").get<std::string>();
		lightingPass = json.at("lightingPass").get<std::string>();
		cullMode = static_cast<CullMode>(json.at("cullMode").get<int>());
		colorMask = json.at("colorMask").get<uint8_t>();
		zwrite = json.at("zwrite").get<bool>();
		bZTest = json.value("ztest", true);

		// stencil
		stencil.Deserialize(json.at("stencil"));

		// constants
		constants.clear();
		if (json.contains("constants"))
		{
			for (const auto& u : json.at("constants"))
			{
				VariableNode var;
				var.Deserialize(u);
				constants.push_back(std::move(var));
			}
		}

		// stages
		stages.clear();
		for (const auto& s : json.at("stages"))
		{
			StageNode stage;
			stage.Deserialize(s);
			stages.push_back(std::move(stage));
		}
	}
	SH_RENDER_API auto ShaderAST::VersionNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["versionNumber"] = versionNumber;
		json["profile"] = profile;
		return json;
	}
	SH_RENDER_API void ShaderAST::VersionNode::Deserialize(const core::Json& json)
	{
		versionNumber = json.at("versionNumber").get<int>();
		profile = json.at("profile").get<std::string>();
	}
	SH_RENDER_API auto ShaderAST::ShaderNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["version"] = version.Serialize();
		json["shaderName"] = shaderName;

		// properties
		core::Json propArray = core::Json::array();
		for (const auto& p : properties)
			propArray.push_back(p.Serialize());
		json["properties"] = std::move(propArray);

		// passes
		core::Json passArray = core::Json::array();
		for (const auto& p : passes)
			passArray.push_back(p.Serialize());
		json["passes"] = std::move(passArray);

		return json;
	}
	SH_RENDER_API void ShaderAST::ShaderNode::Deserialize(const core::Json& json)
	{
		version.Deserialize(json.at("version"));
		shaderName = json.at("shaderName").get<std::string>();

		properties.clear();
		for (const auto& p : json.at("properties"))
		{
			VariableNode var;
			var.Deserialize(p);
			properties.push_back(std::move(var));
		}

		passes.clear();
		for (const auto& p : json.at("passes"))
		{
			PassNode pass;
			pass.Deserialize(p);
			passes.push_back(std::move(pass));
		}
	}
	SH_RENDER_API auto ShaderAST::ComputeShaderNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["version"] = version.Serialize();
		json["shaderName"] = shaderName;
		json["numthreadsX"] = numthreadsX;
		json["numthreadsY"] = numthreadsY;
		json["numthreadsZ"] = numthreadsZ;
		json["code"] = code;

		core::Json bufferArray = core::Json::array();
		for (const auto& b : buffers)
			bufferArray.push_back(b.Serialize());
		json["buffers"] = std::move(bufferArray);

		json["declaration"] = declaration;
		json["functions"] = functions;
		return json;
	}
	SH_RENDER_API void ShaderAST::ComputeShaderNode::Deserialize(const core::Json& json)
	{
		version.Deserialize(json.at("version"));
		shaderName = json.at("shaderName").get<std::string>();
		numthreadsX = json.value("numthreadsX", 1u);
		numthreadsY = json.value("numthreadsY", 1u);
		numthreadsZ = json.value("numthreadsZ", 1u);
		code = json.at("code").get<std::string>();

		buffers.clear();
		for (const auto& b : json.at("buffers"))
		{
			BufferNode buf;
			buf.Deserialize(b);
			buffers.push_back(std::move(buf));
		}
		declaration = json.at("declaration").get<std::vector<std::string>>();
		functions = json.at("functions").get<std::vector<std::string>>();
	}
}//namespace