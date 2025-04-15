#pragma once
#include "Game/Component/Component.h"
#include "Game/Vector.h"

#include "glm/gtc/quaternion.hpp"

#ifdef None
#undef None
#endif

namespace sh::game
{
	class Camera;
	class LineRenderer;

	class EditorControl : public Component
	{
		COMPONENT(EditorControl)
	private:
		enum class Mode
		{
			None, Move, Rotate, Scale
		};
		enum class Axis
		{
			None, X, Y, Z
		};
		Mode mode = Mode::None;
		Axis axis = Axis::None;

		PROPERTY(camera)
		Camera* camera = nullptr;
		PROPERTY(helper, core::PropertyOption::invisible)
		LineRenderer* helper = nullptr;

		Vec3 posLast, scaleLast;
		glm::quat quatLast;
		Vec2 clickPos;

		Vec3 up, right, forward;
	private:
		inline void MoveControl();
		inline void ScaleControl();
		inline void RotateControl();
	public:
		SH_GAME_API EditorControl(GameObject& owner);

		SH_GAME_API void Update() override;

		SH_GAME_API void SetCamera(Camera* camera);

		SH_GAME_API auto Serialize() const -> core::Json override;
	};
}//namespace