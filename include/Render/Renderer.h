#pragma once

#include "Export.h"

#include "../Window/Window.h"

#include "glm/vec2.hpp"

#include <array>
#include <map>
#include <queue>
#include <set>
#include <utility>
#include <functional>
#include <mutex>
#include <atomic>
namespace sh::render {
	enum class RenderAPI
	{
		OpenGL,
		Vulkan
	};
	class IDrawable;
	class Framebuffer;

	class Renderer {
	public:
		using CameraHandle = uint32_t;
		struct Camera
		{
			CameraHandle id;
			int depth;

			bool operator<(const Camera& other) const;
		};
	private:
		sh::window::Window* window;
		uint32_t nextCameraId;
	protected:
		static constexpr int GAME_THREAD = 0;
		static constexpr int RENDER_THREAD = 1;

		std::queue<CameraHandle> emptyHandle;
		//todo
		std::vector<const Camera*> camHandles;
		std::array<std::map<Camera, std::vector<IDrawable*>>, 2> drawList;
		std::vector<std::function<void()>> drawCalls;

		glm::vec2 viewportStart;
		glm::vec2 viewportEnd;

		CameraHandle mainCamera;

		std::mutex drawListMutex;

		std::atomic_bool bPause;
	public:
		const RenderAPI apiType;
	public:
		SH_RENDER_API Renderer(RenderAPI api);
		SH_RENDER_API virtual ~Renderer() = default;

		SH_RENDER_API virtual bool Init(sh::window::Window& win);
		SH_RENDER_API virtual bool Resizing() = 0;
		SH_RENDER_API virtual void Clean() = 0;

		SH_RENDER_API virtual void Render(float deltaTime) = 0;
		SH_RENDER_API virtual void Pause(bool b);

		SH_RENDER_API virtual bool IsInit() const = 0;
		SH_RENDER_API virtual auto GetMainFramebuffer() const -> const Framebuffer* = 0;

		SH_RENDER_API virtual auto GetWidth() const -> uint32_t = 0;
		SH_RENDER_API virtual auto GetHeight() const -> uint32_t = 0;

		SH_RENDER_API virtual void SetViewport(const glm::vec2& start, const glm::vec2& end) = 0;
		SH_RENDER_API auto GetViewportStart() const -> const glm::vec2&;
		SH_RENDER_API auto GetViewportEnd() const -> const glm::vec2&;

		SH_RENDER_API void ClearDrawList();
		/// @brief [게임 스레드 전용] 드로우 객체를 큐에 집어 넣는다.
		/// @param drawable 드로우 객체 포인터
		/// @param camHandle 카메라 핸들
		/// @return 
		SH_RENDER_API void PushDrawAble(IDrawable* drawable, CameraHandle camHandle = 0);
		/// @brief [렌더 스레드 전용] 별도의 드로우 콜을 추가한다.
		/// @param func 드로우 콜 함수
		/// @return 
		SH_RENDER_API void AddDrawCall(const std::function<void()>& func);

		SH_RENDER_API auto GetWindow() -> sh::window::Window&;

		SH_RENDER_API auto AddCamera(int depth = 0) -> CameraHandle;
		SH_RENDER_API void DeleteCamera(CameraHandle cam);
		SH_RENDER_API void SetMainCamera(CameraHandle cam);
		SH_RENDER_API void SetCameraDepth(CameraHandle, int depth);

		/// @brief 메인 스레드와 동기화 하는 함수.
		/// @return 
		SH_RENDER_API void SyncGameThread();

		/// @brief 렌더러가 일시정지 상태인지 반환한다.
		/// @return 일시정지 시 true 그 외 false
		SH_RENDER_API auto IsPause() const -> bool;
	};
}