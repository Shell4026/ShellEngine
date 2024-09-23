﻿#pragma once

#include "Export.h"
#include "Camera.h"

#include "Core/ISyncable.h"
#include "Core/SContainer.hpp"

#include "Window/Window.h"

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
	private:
		sh::window::Window* window;

		std::queue<core::ISyncable*> syncQueue;

		struct CameraCompare
		{
			bool operator()(const Camera* left, const Camera* right) const
			{
				return left->GetPriority() > right->GetPriority();
			}
		};
	protected:
		static constexpr int GAME_THREAD = 0;
		static constexpr int RENDER_THREAD = 1;

		core::SyncArray<std::map<Camera*, core::SVector<IDrawable*>, CameraCompare>> drawList;

		std::vector<std::function<void()>> drawCalls;

		glm::vec2 viewportStart;
		glm::vec2 viewportEnd;

		std::atomic_bool bPause;
	public:
		const RenderAPI apiType;
	public:
		SH_RENDER_API Renderer(RenderAPI api);
		SH_RENDER_API virtual ~Renderer() = default;

		SH_RENDER_API virtual bool Init(sh::window::Window& win);
		SH_RENDER_API virtual bool Resizing() = 0;
		SH_RENDER_API virtual void Clean();

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
		/// @return 
		SH_RENDER_API void PushDrawAble(IDrawable* drawable);
		/// @brief [렌더 스레드 전용] 별도의 드로우 콜을 추가한다.
		/// @param func 드로우 콜 함수
		/// @return 
		SH_RENDER_API void AddDrawCall(const std::function<void()>& func);

		SH_RENDER_API auto GetWindow() -> sh::window::Window&;

		/// @brief 메인 스레드와 동기화 하는 함수.
		/// @return 
		SH_RENDER_API void SyncGameThread();

		/// @brief 렌더러가 일시정지 상태인지 반환한다.
		/// @return 일시정지 시 true 그 외 false
		SH_RENDER_API auto IsPause() const -> bool;

		/// @brief 스레드 간 동기화를 할 객체를 큐에 넣는 함수.
		/// @param syncObj 동기화 객체 참조
		/// @return 
		SH_RENDER_API void PushSyncObject(core::ISyncable& syncObj);
	};
}