#pragma once
#include "Texture.h"

#include "Core/Util.h"

#include <utility>
namespace sh::render
{
	struct RenderTargetLayout
	{
		TextureFormat format;
		TextureFormat depthFormat;
		bool bUseMSAA = false;

		auto operator==(const RenderTargetLayout& other) const -> bool
		{
			return format == other.format && depthFormat == other.depthFormat && bUseMSAA == other.bUseMSAA;
		}
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
			std::hash<int> boolHasher{};

			std::size_t hash = intHasher(static_cast<int>(layout.format));
			hash = sh::core::Util::CombineHash(hash, intHasher(static_cast<int>(layout.depthFormat)));
			hash = sh::core::Util::CombineHash(hash, boolHasher(layout.bUseMSAA));

			return hash;
		}
	};
}
