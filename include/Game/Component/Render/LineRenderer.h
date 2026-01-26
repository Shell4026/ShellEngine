#pragma once
#include "Game/Export.h"
#include "Game/Component/Component.h"
#include "MeshRenderer.h"

#include "Game/Vector.h"

#include "Core/Reflection.hpp"
#include "Core/SContainer.hpp"

#include "Render/Mesh.h"


namespace sh::game
{
	class LineRenderer : public MeshRenderer
	{
		COMPONENT(LineRenderer)
	private:
		PROPERTY(start)
		Vec3 start;
		PROPERTY(end)
		Vec3 end;
		PROPERTY(color)
		Vec4 color;

		render::Mesh mesh;
	public:
		SH_GAME_API LineRenderer(GameObject& owner);

		SH_GAME_API void Awake() override;
		SH_GAME_API void Update() override;

		SH_GAME_API void SetStart(const Vec3& start);
		SH_GAME_API void SetEnd(const Vec3& end);
		SH_GAME_API void SetColor(const Vec4& color);

		SH_GAME_API auto GetStart() const -> const Vec3&;
		SH_GAME_API auto GetEnd() const -> const Vec3&;
		SH_GAME_API auto GetColor() const -> const Vec4&;
	};
}//namespace