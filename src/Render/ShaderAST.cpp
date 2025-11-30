#include "ShaderAST.h"

namespace sh::render
{
	auto ShaderAST::VariableNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["type"] = static_cast<int>(type);
		json["size"] = size;
		json["name"] = name;
		json["default"] = defaultValue;
		json["attribute"] = static_cast<int>(attribute);
		return json;
	}

	void ShaderAST::VariableNode::Deserialize(const core::Json& json)
	{
		type = static_cast<VariableType>(json.at("type").get<int>());
		size = json.at("size").get<int>();
		name = json.at("name").get<std::string>();
		if (json.contains("default"))
			defaultValue = json["default"].get<std::string>();
		attribute = static_cast<VariableAttribute>(json.at("attribute").get<int>());
	}

	auto ShaderAST::LayoutNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["binding"] = binding;
		json["var"] = var.Serialize();
		return json;
	}

	void ShaderAST::LayoutNode::Deserialize(const core::Json& json)
	{
		binding = json.at("binding").get<uint32_t>();
		var.Deserialize(json.at("var"));
	}

	auto ShaderAST::UBONode::Serialize() const -> core::Json
	{
		core::Json json;
		json["set"] = set;
		json["binding"] = binding;
		json["name"] = name;
		json["sampler"] = bSampler;
		json["constant"] = bConstant;

		core::Json varArray = core::Json::array();
		for (const auto& v : vars)
		{
			varArray.push_back(v.Serialize());
		}
		json["vars"] = std::move(varArray);
		return json;
	}

	void ShaderAST::UBONode::Deserialize(const core::Json& json)
	{
		set = json.at("set").get<uint32_t>();
		binding = json.at("binding").get<uint32_t>();
		name = json.at("name").get<std::string>();
		bSampler = json.at("sampler").get<bool>();
		bConstant = json.at("constant").get<bool>();

		vars.clear();
		for (const auto& v : json.at("vars"))
		{
			VariableNode var;
			var.Deserialize(v);
			vars.push_back(std::move(var));
		}
	}

	auto ShaderAST::StageNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["type"] = static_cast<int>(type);
		json["bUseLighting"] = bUseLighting;
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

		// uniforms
		core::Json uboArray = core::Json::array();
		for (const auto& u : uniforms)
			uboArray.push_back(u.Serialize());
		json["uniforms"] = std::move(uboArray);

		// declaration
		json["declaration"] = declaration;

		// functions
		json["functions"] = functions;

		return json;
	}

	void ShaderAST::StageNode::Deserialize(const core::Json& json)
	{
		type = static_cast<StageType>(json.at("type").get<int>());
		bUseLighting = json.at("bUseLighting").get<bool>();
		code = json.at("code").get<std::string>();

		// in
		in.clear();
		for (const auto& i : json.at("in"))
		{
			LayoutNode l;
			l.Deserialize(i);
			in.push_back(std::move(l));
		}

		// out
		out.clear();
		for (const auto& o : json.at("out"))
		{
			LayoutNode l;
			l.Deserialize(o);
			out.push_back(std::move(l));
		}

		// uniforms
		uniforms.clear();
		for (const auto& u : json.at("uniforms"))
		{
			UBONode ubo;
			ubo.Deserialize(u);
			uniforms.push_back(std::move(ubo));
		}

		// declaration
		declaration = json.at("declaration").get<std::vector<std::string>>();

		// functions
		functions = json.at("functions").get<std::vector<std::string>>();
	}

	auto ShaderAST::PassNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["name"] = name;
		json["lightingPass"] = lightingPass;
		json["cullMode"] = static_cast<int>(cullMode);
		json["colorMask"] = colorMask;
		json["zwrite"] = zwrite;

		// stencil
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

	void ShaderAST::PassNode::Deserialize(const core::Json& json)
	{
		name = json.at("name").get<std::string>();
		lightingPass = json.at("lightingPass").get<std::string>();
		cullMode = static_cast<CullMode>(json.at("cullMode").get<int>());
		colorMask = json.at("colorMask").get<uint8_t>();
		zwrite = json.at("zwrite").get<bool>();

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

	auto ShaderAST::VersionNode::Serialize() const -> core::Json
	{
		core::Json json;
		json["versionNumber"] = versionNumber;
		json["profile"] = profile;
		return json;
	}

	void ShaderAST::VersionNode::Deserialize(const core::Json& json)
	{
		versionNumber = json.at("versionNumber").get<int>();
		profile = json.at("profile").get<std::string>();
	}

	auto ShaderAST::ShaderNode::Serialize() const -> core::Json
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

	void ShaderAST::ShaderNode::Deserialize(const core::Json& json)
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

}//namespace