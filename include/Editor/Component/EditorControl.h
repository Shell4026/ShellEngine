#pragma once
#include "../Export.h"

#include "Core/EventSubscriber.h"

#include "Game/Component/Component.h"
#include "Game/Vector.h"
#include "Game/WorldEvents.hpp"

#include "glm/gtc/quaternion.hpp"

#ifdef None
#undef None
#endif

#include <cstdint>
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
	public:
		SH_EDITOR_API EditorControl(game::GameObject& owner);
		SH_EDITOR_API ~EditorControl();

		SH_EDITOR_API void Awake() override;
		SH_EDITOR_API void Update() override;
		SH_EDITOR_API void LateUpdate() override;

		SH_EDITOR_API void SetCamera(game::Camera* camera);

		SH_EDITOR_API auto Serialize() const -> core::Json override;

		SH_EDITOR_API static void SetSnap(bool bSnap);
		SH_EDITOR_API static void SetSnapDistance(float dis);
		SH_EDITOR_API static void SetSnapAngle(float degree);

		SH_EDITOR_API static auto GetSnap() -> bool { return bSnap; }
		SH_EDITOR_API static auto GetSnapDistance() -> float { return snapDis; }
		SH_EDITOR_API static auto GetSnapAngle() -> float { return snapAngle; }
	private:
		enum class Mode
		{
			None, Move, Rotate, Scale
		};
		struct Snapshot
		{
			game::Vec3 position;
			game::Vec3 scale;
			glm::quat rotation;
		};

		void MoveControl();
		void ScaleControl();
		void RotateControl();

		void BeginManipulation(Mode nextMode);
		void CompleteManipulation(bool bCancel);
		void ResetManipulationState();
		auto HasTransformChanged() const -> bool;
		auto CreateSnapshot() const -> Snapshot;
		static auto GetCommandName(Mode mode) -> const char*;
		static auto GetTransactionName(Mode mode) -> const char*;
	private:
		Mode mode = Mode::None;
		enum class Axis
		{
			None, X, Y, Z
		} axis = Axis::None;

		PROPERTY(camera)
		game::Camera* camera = nullptr;
		PROPERTY(helper, core::PropertyOption::invisible)
		game::LineRenderer* helper = nullptr;

		EditorUI* ui = nullptr;

		game::Vec3 posLast, scaleLast;
		glm::quat quatLast;
		game::Vec2 clickPos;
		bool bParticipatingManipulation = false;

		game::Vec3 up, right, forward;

		core::EventSubscriber<game::events::WorldEvent> worldEventSubscriber;

		static std::set<EditorControl*> controls;
		static float snapDis;
		static float snapAngle;
		static bool bPivot;
		static bool bUpdatedControls;
		static bool bSnap;

		static uint32_t activeManipulationCount;
		static bool bOwnsHistoryTransaction;
		static bool bCancelHistoryTransaction;
	};
}//namespace