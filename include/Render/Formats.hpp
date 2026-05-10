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

	inline static auto GetTextureFormatChannel(TextureFormat format) -> int
	{
		switch (format)
		{
		case TextureFormat::R8:
			return 1;
		}
		return 4;
	}

	inline static auto IsDepthTexture(TextureFormat format) -> bool
	{
		if (format == TextureFormat::D32S8 ||
			format == TextureFormat::D24S8 ||
			format == TextureFormat::D16S8 ||
			format == TextureFormat::D32 ||
			format == TextureFormat::D16)
			return true;
		return false;
	}

	inline static auto TextureFormatToString(TextureFormat format) -> const char*
	{
		switch (format)
		{
		case TextureFormat::None: return "None";
		case TextureFormat::SRGB24: return "SRGB24";
		case TextureFormat::SRGBA32: return "SRGBA32";
		case TextureFormat::SBGR24: return "SBGR24";
		case TextureFormat::SBGRA32: return "SBGRA32";
		case TextureFormat::RGB24: return "RGB24";
		case TextureFormat::RGBA32: return "RGBA32";
		case TextureFormat::R8: return "R8";
		case TextureFormat::D32S8: return "D32S8";
		case TextureFormat::D24S8: return "D24S8";
		case TextureFormat::D16S8: return "D16S8";
		case TextureFormat::D32: return "D32";
		case TextureFormat::D16: return "D16";
		default: return "None";
		}
	}
}//namespace