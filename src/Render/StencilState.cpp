#include "StencilState.h"

namespace sh::render
{
	SH_RENDER_API auto StencilState::Serialize() const -> core::Json
	{
		core::Json json{};

		json["ref"] = ref;
		json["compareMask"] = compareMask;
		json["writeMask"] = writeMask;
		json["compareOp"] = static_cast<int>(compareOp);
		json["passOp"] = static_cast<int>(passOp);
		json["failOp"] = static_cast<int>(failOp);
		json["depthFailOp"] = static_cast<int>(depthFailOp);

		return json;
	}
	SH_RENDER_API void StencilState::Deserialize(const core::Json& json)
	{
		if (json.contains("ref"))
			ref = json["ref"];
		if (json.contains("compareMask"))
			compareMask = json["compareMask"];
		if (json.contains("writeMask"))
			writeMask = json["writeMask"];
		if (json.contains("compareOp"))
			compareOp = static_cast<CompareOp>(json["compareOp"]);
		if (json.contains("passOp"))
			passOp = static_cast<StencilOp>(json["passOp"]);
		if (json.contains("failOp"))
			failOp = static_cast<StencilOp>(json["failOp"]);
		if (json.contains("depthFailOp"))
			depthFailOp = static_cast<StencilOp>(json["depthFailOp"]);
	}
}//namespace