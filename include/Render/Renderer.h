#pragma once

#include "Export.h"
#include "Camera.h"
#include "RenderPipeline.h"

#include "Core/ISyncable.h"
#include "Core/SContainer.hpp"

#include "glm/vec2.hpp"

#include <vector>
#include <memory>
#include <array>
#include <map>
#include <queue>
#include <set>
#include <utility>
#include <functional>
#include <thread>
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
		SCLASS(Renderer)
	public:
		static constexpr int SYNC_PRIORITY = -10000;
	public:
		SH_RENDER_API Renderer();
		SH_RENDER_API virtual ~Renderer();

		SH_RENDER_API virtual bool Init(sh::window::Window& win);
		SH_RENDER_API virtual bool Resizing() = 0;
		SH_RENDER_API virtual void Clear();

		SH_RENDER_API virtual void Render();
		SH_RENDER_API virtual void Pause(bool b);

		SH_RENDER_API virtual bool IsInit() const = 0;

		SH_RENDER_API virtual auto GetWidth() const -> uint32_t = 0;
		SH_RENDER_API virtual auto GetHeight() const -> uint32_t = 0;
		SH_RENDER_API virtual void WaitForCurrentFrame() = 0;

		/// @brief [게임 스레드 전용] 드로우 객체를 큐에 집어 넣는다.
		/// @brief 큐에 들어간 객체는 sync 타이밍에 렌더러에 들어간다.
		/// @param drawable 드로우 객체 포인터
		SH_RENDER_API void PushDrawAble(Drawable* drawable);
		/// @brief [렌더 스레드 전용] 별도의 드로우 콜을 추가한다.
		/// @param func 드로우 콜 함수
		SH_RENDER_API void AddDrawCall(const std::function<void()>& func);

		SH_RENDER_API auto GetWindow() const -> sh::window::Window&;

		/// @brief 렌더러가 일시정지 상태인지 반환한다.
		/// @return 일시정지 시 true 그 외 false
		SH_RENDER_API auto IsPause() const -> bool;

		SH_RENDER_API void SyncDirty() override;
		SH_RENDER_API void Sync() override;

		SH_RENDER_API virtual auto GetContext() const -> IRenderContext* = 0;

		SH_RENDER_API auto AddRenderPipeline(std::unique_ptr<RenderPipeline>&& renderPipeline) -> RenderPipeline*;
		              template<typename T, typename Check = std::enable_if_t<std::is_base_of_v<RenderPipeline, T>>>
		              auto AddRenderPipeline() -> T*;
		SH_RENDER_API void ClearRenderPipeline();
		SH_RENDER_API auto GetRenderPipeline(const core::Name& name) const -> RenderPipeline*;
		SH_RENDER_API auto GetRenderPipelines() -> std::vector<std::unique_ptr<RenderPipeline>>&;

		/// @brief 카메라를 추가한다. 동기화 타이밍 때 추가 된다.
		/// @param camera 카메라 참조
		SH_RENDER_API void AddCamera(const Camera& camera);
		/// @brief 카메라를 제거한다. 동기화 타이밍 때 제거 된다.
		/// @param camera 카메라 참조
		SH_RENDER_API void RemoveCamera(const Camera& camera);

		SH_RENDER_API auto GetDrawCall(core::ThreadType thread) const -> uint32_t;

		/// @brief 현재 렌더러가 돌아가는 스레드의 번호를 반환한다. 한번이라도 렌더링을 한 후에 갱신된다.
		SH_RENDER_API auto GetThreadId() const -> std::thread::id;
	protected:
		virtual void OnCameraAdded(const Camera* camera) {};
		virtual void OnCameraRemoved(const Camera* camera) {};

		void SetDrawCallCount(uint32_t drawcall);
	protected:
		struct CameraCompare
		{
			bool operator()(const Camera* left, const Camera* right) const
			{
				if (left->GetPriority() == right->GetPriority())
					return left->id < right->id;
				return left->GetPriority() < right->GetPriority();
			}
		};
		struct CameraProcess
		{
			const Camera* cameraPtr;
			enum class Mode
			{
				Create,
				Destroy
			} mode;
		};
		std::queue<Drawable*> drawableQueue;
		std::set<const Camera*, CameraCompare> cameras;
		std::vector<std::function<void()>> drawCalls;
		std::vector<std::unique_ptr<RenderPipeline>> renderPipelines;

		std::atomic_bool bPause;
	private:
		sh::window::Window* window;
		std::queue<CameraProcess> cameraQueue;

		core::SyncArray<uint32_t> drawcall;

		std::thread::id threadId;

		bool bFirstRender = false;
		bool bDirty;
		bool bDrawCallDirty = false;
	};

	template<typename T, typename Check>
	inline auto Renderer::AddRenderPipeline() -> T*
	{
		std::unique_ptr<RenderPipeline> pass{ new T{} };
		pass->Create(*GetContext());

		auto ptr = pass.get();

		renderPipelines.push_back(std::move(pass));

		return static_cast<T*>(ptr);
	}
}