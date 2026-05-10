#pragma once
#include "Mesh.h"
#include "Texture.h"
#include "IRenderThrMethod.h"
#include "Formats.hpp"

#include "Core/SContainer.hpp"
#include "Core/Name.h"

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <variant>
#include <vector>

namespace sh
{
	template<typename T>
	using Ref = std::reference_wrapper<T>;
}
namespace sh::render
{
	class Camera;
	class RenderTexture;
	class Drawable;
	class Material;
	class CommandBuffer;

	struct RenderViewer
	{
		glm::mat4 viewMatrix;
		glm::mat4 projMatrix;
		glm::vec3 pos;
		glm::vec3 to;

		glm::uvec4 viewportRect;
		glm::uvec4 viewportScissor;
		std::size_t offset = 0;
	};

	class RenderData
	{
		friend struct IRenderThrMethod<RenderData>;
	public:
		void SetRenderTarget(const RenderTexture* renderTarget) { targets.clear(); targets.push_back(renderTarget); }
		void SetRenderTargets(std::initializer_list<const RenderTexture*> renderTargets)
		{
			if (renderTargets.size() == 0)
			{
				targets.clear();
				targets.push_back(nullptr);
			}
			else
				targets.assign(renderTargets.begin(), renderTargets.end());
		}
		void SetRenderTargets(std::vector<const RenderTexture*> renderTargets)
		{
			if (renderTargets.size() == 0)
			{
				targets.clear();
				targets.push_back(nullptr);
			}
			else
				targets = std::move(renderTargets);
		}
		void ClearRenderTargets() { targets.clear(); targets.push_back(nullptr); }
		auto GetRenderTarget(std::size_t idx) const -> const RenderTexture* { return idx < targets.size() ? targets[idx] : nullptr; }
		auto GetRenderTargets() const -> const std::vector<const RenderTexture*>& { return targets; }
		auto GetFrameIdx() const -> uint32_t { return frameIndex; }
		auto GetDrawablesPtr() const -> const std::vector<Drawable*>* { return drawables; }
	public:
		int priority = 0;
		core::Name tag{ "Camera" };
		std::vector<RenderViewer> renderViewers;
	private:
		uint32_t frameIndex = 0;
		std::vector<const RenderTexture*> targets{ nullptr };
		const std::vector<Drawable*>* drawables = nullptr;
	};

	template<>
	struct IRenderThrMethod<RenderData>
	{
		inline static void SetFrameIndex(RenderData& rd, uint32_t frameIdx) { rd.frameIndex = frameIdx; }
		inline static void SetDrawablesPtr(RenderData& rd, const std::vector<Drawable*>* ptr) { rd.drawables = ptr; }
	};

	class RenderTargetLayout
	{
	public:
		auto operator==(const RenderTargetLayout& other) const -> bool
		{
			return colorFormats == other.colorFormats && depthFormat == other.depthFormat && bUseMSAA == other.bUseMSAA;
		}
		auto IsDepthOnly() const -> bool
		{
			for (TextureFormat format : colorFormats)
			{
				if (format != TextureFormat::None)
					return false;
			}
			return depthFormat != TextureFormat::None;
		}
	public:
		std::vector<TextureFormat> colorFormats{ TextureFormat::None };
		TextureFormat depthFormat = TextureFormat::None;

		bool bUseMSAA = false;
	};

	/// @brief 리소스(이미지/버퍼) 사용 의도. 배리어 계산의 입력으로 쓰임
	enum class ResourceUsage
	{
		Undefined,
		ColorAttachment,
		SampledRead,
		Present,
		TransferSrc,
		DepthStencilAttachment,
		DepthStencilSampledRead
	};

	struct BarrierInfo
	{
		std::variant<const RenderTexture*, uint32_t> target; // uint32_t = 스왑체인
		ResourceUsage lastUsage;
		ResourceUsage curUsage;
	};
}//namespace

namespace std
{
	template<>
	struct hash<sh::render::RenderTargetLayout>
	{
		auto operator()(const sh::render::RenderTargetLayout& layout) const -> std::size_t
		{
			std::hash<int> intHasher{};
			std::hash<bool> boolHasher{};

			std::size_t hash = intHasher(static_cast<int>(layout.depthFormat));
			hash = sh::core::Util::CombineHash(hash, boolHasher(layout.bUseMSAA));
			for (std::size_t i = 0; i < layout.colorFormats.size(); ++i)
				hash = sh::core::Util::CombineHash(hash, intHasher(static_cast<int>(layout.colorFormats[i])));
			return hash;
		}
	};
}//namespace std
