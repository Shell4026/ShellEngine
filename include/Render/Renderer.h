﻿#pragma once

#include "Export.h"
#include "Camera.h"

#include "Core/ISyncable.h"
#include "Core/SContainer.hpp"

#include "glm/vec2.hpp"

#include <array>
#include <map>
#include <queue>
#include <set>
#include <utility>
#include <functional>
namespace sh::window
{
	class Window;
}
namespace sh::render 
{
	class IDrawable;
	class Framebuffer;
	class IRenderContext;

	class Renderer : public core::ISyncable
	{
		SCLASS(Renderer)
	private:
		const sh::window::Window* window;

		struct CameraCompare
		{
			bool operator()(const Camera* left, const Camera* right) const
			{
				if (left->GetPriority() == right->GetPriority())
					return left->id < right->id;
				return left->GetPriority() < right->GetPriority();
			}
		};
	protected:
		std::queue<IDrawable*> drawableQueue;
		core::SyncArray<std::map<Camera*, core::SVector<IDrawable*>, CameraCompare>> drawList;

		std::vector<std::function<void()>> drawCalls;

		glm::vec2 viewportStart;
		glm::vec2 viewportEnd;

		std::atomic_bool bPause;
	private:
		bool bDirty;
	public:
		SH_RENDER_API Renderer();
		SH_RENDER_API virtual ~Renderer() = default;

		SH_RENDER_API virtual bool Init(const sh::window::Window& win);
		SH_RENDER_API virtual bool Resizing() = 0;
		SH_RENDER_API virtual void Clean();

		SH_RENDER_API virtual void Render(float deltaTime) = 0;
		SH_RENDER_API virtual void Pause(bool b);

		SH_RENDER_API virtual bool IsInit() const = 0;

		SH_RENDER_API virtual auto GetWidth() const -> uint32_t = 0;
		SH_RENDER_API virtual auto GetHeight() const -> uint32_t = 0;

		SH_RENDER_API virtual void SetViewport(const glm::vec2& start, const glm::vec2& end);
		SH_RENDER_API auto GetViewportStart() const -> const glm::vec2&;
		SH_RENDER_API auto GetViewportEnd() const -> const glm::vec2&;

		SH_RENDER_API void ClearDrawList();
		/// @brief [게임 스레드 전용] 드로우 객체를 큐에 집어 넣는다.
		/// @brief 큐에 들어간 객체는 sync 타이밍에 렌더러에 들어간다.
		/// @param drawable 드로우 객체 포인터
		/// @return 
		SH_RENDER_API void PushDrawAble(IDrawable* drawable);
		/// @brief [렌더 스레드 전용] 별도의 드로우 콜을 추가한다.
		/// @param func 드로우 콜 함수
		/// @return 
		SH_RENDER_API void AddDrawCall(const std::function<void()>& func);

		SH_RENDER_API auto GetWindow() const -> const sh::window::Window&;

		/// @brief 렌더러가 일시정지 상태인지 반환한다.
		/// @return 일시정지 시 true 그 외 false
		SH_RENDER_API auto IsPause() const -> bool;

		SH_RENDER_API void SetDirty() override;
		SH_RENDER_API void Sync() override;

		SH_RENDER_API virtual auto GetContext() const -> IRenderContext* = 0;
	};
}