#pragma once

#include "Export.h"
#include "Camera.h"
#include "ILightingPass.h"

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
		struct CameraProcess
		{
			const Camera* cameraPtr;
			enum class Mode
			{
				Create,
				Destroy
			} mode;
		};

		std::queue<CameraProcess> cameraQueue;
	protected:
		std::queue<Drawable*> drawableQueue;
		std::set<const Camera*, CameraCompare> cameras;
		std::vector<std::function<void()>> drawCalls;
		std::vector<std::unique_ptr<ILightingPass>> lightingPasses;

		std::atomic_bool bPause;
	private:
		bool bDirty;
	protected:
		virtual void OnCameraAdded(const Camera* camera) {};
		virtual void OnCameraRemoved(const Camera* camera) {};
	public:
		SH_RENDER_API Renderer();
		SH_RENDER_API virtual ~Renderer() = default;

		SH_RENDER_API virtual bool Init(const sh::window::Window& win);
		SH_RENDER_API virtual bool Resizing() = 0;
		SH_RENDER_API virtual void Clear();

		SH_RENDER_API virtual void Render() = 0;
		SH_RENDER_API virtual void Pause(bool b);

		SH_RENDER_API virtual bool IsInit() const = 0;

		SH_RENDER_API virtual auto GetWidth() const -> uint32_t = 0;
		SH_RENDER_API virtual auto GetHeight() const -> uint32_t = 0;

		/// @brief [게임 스레드 전용] 드로우 객체를 큐에 집어 넣는다.
		/// @brief 큐에 들어간 객체는 sync 타이밍에 렌더러에 들어간다.
		/// @param drawable 드로우 객체 포인터
		SH_RENDER_API void PushDrawAble(Drawable* drawable);
		/// @brief [렌더 스레드 전용] 별도의 드로우 콜을 추가한다.
		/// @param func 드로우 콜 함수
		SH_RENDER_API void AddDrawCall(const std::function<void()>& func);

		SH_RENDER_API auto GetWindow() const -> const sh::window::Window&;

		/// @brief 렌더러가 일시정지 상태인지 반환한다.
		/// @return 일시정지 시 true 그 외 false
		SH_RENDER_API auto IsPause() const -> bool;

		SH_RENDER_API void SetDirty() override;
		SH_RENDER_API void Sync() override;

		SH_RENDER_API virtual auto GetContext() const -> IRenderContext* = 0;

		SH_RENDER_API void AddRenderPass(std::unique_ptr<ILightingPass>&& renderPass);
		              template<typename T, typename Check = std::enable_if_t<std::is_base_of_v<ILightingPass, T>>>
		              void AddRenderPass();
		SH_RENDER_API void ClearRenderPass();

		/// @brief 카메라를 추가한다. 동기화 타이밍 때 추가 된다.
		/// @param camera 카메라 참조
		SH_RENDER_API void AddCamera(const Camera& camera);
		/// @brief 카메라를 제거한다. 동기화 타이밍 때 제거 된다.
		/// @param camera 카메라 참조
		SH_RENDER_API void RemoveCamera(const Camera& camera);
	};

	template<typename T, typename Check>
	inline void Renderer::AddRenderPass()
	{
		std::unique_ptr<ILightingPass> pass = std::make_unique<T>();
		pass->Init(*GetContext());
		lightingPasses.push_back(std::move(pass));
	}
}