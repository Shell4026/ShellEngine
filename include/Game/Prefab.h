#pragma once
#include "Export.h"

#include "Core/SObject.h"

#include <regex>
namespace sh::game
{
	class World;
	class GameObject;

	class Prefab : public core::SObject
	{
		SCLASS(Prefab)
	public:
		SH_GAME_API auto AddToWorld(World& world) -> GameObject*;

		SH_GAME_API auto Serialize() const->core::Json override;
		SH_GAME_API void Deserialize(const core::Json& json) override;

		SH_GAME_API static auto CreatePrefab(const GameObject& obj) -> Prefab*;
	private:
		void ChangeUUIDS(const std::unordered_map<std::string, std::string>& changed, core::Json& json);
	private:
		core::Json prefabJson;
	};
}//namespace