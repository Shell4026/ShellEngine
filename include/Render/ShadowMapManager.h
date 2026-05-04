#pragma once
#include "Export.h"
#include "RenderData.h"

#include "Core/NonCopyable.h"

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace sh::render
{
	class IRenderContext;
	class RenderTexture;
	class Renderer;
	class ShelfPacker;

	/// @brief 그림자 캐스터가 구현하는 인터페이스
	class IShadowCaster
	{
	public:
		virtual ~IShadowCaster() = default;
		virtual auto GetShadowMapResolution() const -> uint32_t = 0;
		virtual auto GetShadowViewMatrix() const -> glm::mat4 = 0;
		virtual auto GetShadowProjMatrix() const -> glm::mat4 = 0;
		virtual auto GetShadowPos() const -> glm::vec3 = 0;
		virtual auto GetShadowLookAt() const -> glm::vec3 = 0;
	};

	/// @brief 월드 단위의 그림자 아틀라스 매니저. 스레드 안전하다.
	class ShadowMapManager : public core::INonCopyable
	{
	public:
		struct Slot
		{
			glm::vec2 uvOffset{ 0.f, 0.f };
			glm::vec2 uvSize{ 0.f, 0.f };
			bool valid = false;
		};
	public:
		SH_RENDER_API ShadowMapManager();
		SH_RENDER_API ~ShadowMapManager();

		SH_RENDER_API void Init(IRenderContext& ctx, uint32_t atlasSize = 4096);
		/// @brief 등록된 캐스터/슬롯/아틀라스를 해제한다.
		SH_RENDER_API void Clear();

		/// @brief 그림자 캐스터를 등록한다. 같은 객체의 중복 등록은 무시된다.
		SH_RENDER_API void Register(IShadowCaster& caster);
		SH_RENDER_API void Unregister(IShadowCaster& caster);

		/// @brief 등록된 캐스터들을 모아 RenderData를 만들어 Renderer에 푸시한다.
		/// @brief 매 프레임 1회 호출하면 된다. 슬롯은 이 시점에 ShelfPacker로 새로 패킹된다.
		SH_RENDER_API void Submit(Renderer& renderer);

		/// @brief 캐스터에 할당된 슬롯 정보를 반환한다.
		/// @brief Submit 호출 후 유효하다. 매 프레임 슬롯 위치가 바뀔 수 있으므로 매번 조회한다.
		SH_RENDER_API auto GetSlot(const IShadowCaster& caster) const -> Slot;
		/// @brief 캐스터의 광원 공간 변환 행렬(proj * view)을 반환한다.
		/// @brief Submit 호출 후 유효하다.
		SH_RENDER_API auto GetLightSpaceMatrix(const IShadowCaster& caster) const -> glm::mat4;

		auto GetAtlas() const -> RenderTexture* { return atlas; }
		auto GetAtlasSize() const -> uint32_t { return atlasSize; }
	private:
		void EnsureAtlas();
	private:
		IRenderContext* ctx = nullptr;
		uint32_t atlasSize = 4096;
		RenderTexture* atlas = nullptr;

		std::unique_ptr<ShelfPacker> packer;

		std::vector<IShadowCaster*> casters;
		std::unordered_map<const IShadowCaster*, Slot> casterSlots;
		std::unordered_map<const IShadowCaster*, glm::mat4> casterLightSpace;

		RenderData renderData;
	};
}//namespace
