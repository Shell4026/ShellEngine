#pragma once
#include "Export.h"

#include "Core/EventSubscriber.h"

#include "Game/Component/Component.h"
#include "Game/Vector.h"
#include "Game/WorldEvents.hpp"

#include "glm/gtc/quaternion.hpp"

#ifdef None
#undef None
#endif

#include <set>

namespace sh::game
{
	class Camera;
	class LineRenderer;
}
namespace sh::editor
{
	class EditorUI;
	class EditorControl : public game::Component
	{
		COMPONENT(EditorControl, "Editor")
	private:
		inline void MoveControl();
		inline void ScaleControl();
		inline void RotateControl();
	public:
		SH_EDITOR_API EditorControl(game::GameObject& owner);
		SH_EDITOR_API ~EditorControl();

		SH_EDITOR_API void Awake() override;
		SH_EDITOR_API void Update() override;
		SH_EDITOR_API void LateUpdate() override;

		SH_EDITOR_API void SetCamera(game::Camera* camera);

		SH_EDITOR_API auto Serialize() const -> core::Json override;

		SH_EDITOR_API static void SetSnap(bool bSnap);
		SH_EDITOR_API static auto GetSnap() -> bool;
		SH_EDITOR_API static void SetSnapDistance(float dis);
		SH_EDITOR_API static auto GetSnapDistance() -> float;
		SH_EDITOR_API static void SetSnapAngle(float degree);
		SH_EDITOR_API static auto GetSnapAngle() -> float;
	private:
		static std::set<EditorControl*> controls;
		static float snapDis;
		static float snapAngle;
		static bool bPivot;
		static bool bUpdatedControls;
		static bool bSnap;

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
		game::Camera* camera = nullptr;
		PROPERTY(helper, core::PropertyOption::invisible)
		game::LineRenderer* helper = nullptr;

		EditorUI* ui = nullptr;

		game::Vec3 posLast, scaleLast;
		glm::quat quatLast;
		game::Vec2 clickPos;

		game::Vec3 up, right, forward;

		core::EventSubscriber<game::events::WorldEvent> worldEventSubscriber;
	};
}//namespace