#pragma once
#include "Export.h"
#include "ScriptableRenderer.h"

#include "Core/ISyncable.h"
#include "Core/LockFreeMPSCQueue.h"
#include "Core/SContainer.hpp"

#include "glm/vec2.hpp"

#include <vector>
#include <memory>
#include <array>
#include <map>
#include <set>
#include <utility>
#include <functional>
#include <thread>
#include <variant>
namespace sh::window
{
	class Window;
}
namespace sh::render 
{
	class Drawable;
	class Framebuffer;
	class IRenderContext;

	class Renderer : public core::ISyncable
	{
		friend IRenderThrMethod<Renderer>;
		SCLASS(Renderer)
	public:
		static constexpr int SYNC_PRIORITY = -10000;
	public:
		SH_RENDER_API Renderer();
		SH_RENDER_API virtual ~Renderer();

		SH_RENDER_API virtual void CreateContext(const window::Window& win) = 0;
		SH_RENDER_API virtual void DestroyContext() = 0;
		SH_RENDER_API virtual bool Init(window::Window& win);
		SH_RENDER_API virtual bool Resizing() = 0;
		/// @brief 렌더러 상태를 초기화 한다.
		SH_RENDER_API virtual void Reset();
		SH_RENDER_API virtual void Clear();

		SH_RENDER_API virtual void Render();
		SH_RENDER_API virtual void Pause(bool b);

		SH_RENDER_API virtual bool IsInit() const = 0;

		SH_RENDER_API virtual auto GetWidth() const -> uint32_t = 0;
		SH_RENDER_API virtual auto GetHeight() const -> uint32_t = 0;
		SH_RENDER_API virtual void WaitForCurrentFrame() = 0;
		SH_RENDER_API virtual auto GetContext() const -> IRenderContext* = 0;

		/// @brief 드로우 객체를 큐에 집어 넣는다.
		/// @brief 큐에 들어간 객체는 렌더 스레드 프레임 시작 시 반영된다.
		/// @param drawable 드로우 객체 포인터
		SH_RENDER_API void PushDrawAble(Drawable* drawable);
		SH_RENDER_API void PushRenderData(const RenderData& renderData);

		SH_RENDER_API auto GetWindow() const -> sh::window::Window&;

		/// @brief 렌더러가 일시정지 상태인지 반환한다.
		/// @return 일시정지 시 true 그 외 false
		SH_RENDER_API auto IsPause() const -> bool;

		SH_RENDER_API void SetScriptableRenderer(ScriptableRenderer& renderer);

		auto GetDrawCall(core::ThreadType thread) const -> uint32_t { return drawcall[static_cast<uint32_t>(thread)]; }
		/// @brief 현재 렌더러가 돌아가는 스레드의 번호를 반환한다. 한번이라도 렌더링을 한 후에 갱신된다.
		auto GetThreadId() const -> std::thread::id { return threadId; }
		auto GetScriptableRenderer() const -> ScriptableRenderer* { return renderer; }
	protected:
		SH_RENDER_API void SyncDirty() override;
		SH_RENDER_API void Sync() override;

		SH_RENDER_API void SetDrawCallCount(uint32_t drawcall);
		SH_RENDER_API void DrainRenderCommands();
	protected:
		struct RenderCommand
		{
			std::variant<core::SObjWeakPtr<Drawable>, ScriptableRenderer*> data;
		};

		core::LockFreeMPSCQueue<RenderCommand> renderCommands;
		std::vector<Drawable*> drawables;

		std::atomic_bool bPause;

		ScriptableRenderer* renderer = nullptr;
	private:
		window::Window* window;

		core::SyncArray<uint32_t> drawcall;

		std::thread::id threadId;

		bool bFirstRender = false;
		bool bDirty;
		bool bDrawCallDirty = false;
	};
}//namespace
