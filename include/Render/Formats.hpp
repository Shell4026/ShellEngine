#pragma once

namespace sh::render
{
	enum class TextureFormat
	{
		None,
		SRGB24,
		SRGBA32,
		SBGR24,
		SBGRA32,
		RGB24,
		RGBA32,
		BGR24,
		BGRA32,
		R8,
		D32S8,
		D24S8,
		D16S8,
		D32,
		D16
	};

	static auto GetTextureFormatChannel(TextureFormat format) -> int
	{
		switch (format)
		{
		case TextureFormat::R8:
			return 1;
		}
		return 4;
	}

	static auto IsDepthTexture(TextureFormat format) -> bool
	{
		if (format == TextureFormat::D32S8 ||
			format == TextureFormat::D24S8 ||
			format == TextureFormat::D16S8 ||
			format == TextureFormat::D32 ||
			format == TextureFormat::D16)
			return true;
		return false;
	}
}//namespace