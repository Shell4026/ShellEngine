#pragma once
#include "Export.h"
#include "UUID.h"
#include "Factory.hpp"

#include <vector>
#include <filesystem>
#include <array>

#define SASSET(className, assetType)\
	struct _AssetRegistry_##className\
	{\
		_AssetRegistry_##className()\
		{\
			sh::core::Factory<Asset>::GetInstance()->Register(assetType, [] { return std::make_unique<className>(); });\
		}\
	} static inline assetRegistry{};

namespace sh::core
{
	// 엔진에서 사용 되는 에셋 추상 클래스
	class Asset
	{
		friend class AssetExporter;
		friend class AssetImporter;
	private:
		struct Header 
		{
			char type[4];
			uint32_t version;
			std::array<uint32_t, 4> uuid;
			uint64_t originalDataSize;
			uint64_t compressedDataSize;
			int64_t writeTime;
		};
		const char* type;
		uint32_t assetVersion = 0;
		int64_t writeTime = 0;
	protected:
		UUID assetUUID;
		mutable std::vector<uint8_t> data;
	public:
		constexpr static uint32_t VERSION = 2;
	protected:
		SH_CORE_API explicit Asset(const char* type);

		SH_CORE_API virtual void SetAssetData() const = 0;
		SH_CORE_API virtual auto ParseAssetData() -> bool = 0;
	public:
		SH_CORE_API Asset(const Asset& other);
		SH_CORE_API Asset(Asset&& other) noexcept;
		SH_CORE_API virtual ~Asset() = default;

		SH_CORE_API virtual void SetAsset(const core::SObject& obj) = 0;

		SH_CORE_API auto GetVersion() const -> uint32_t;
		/// @brief 에셋의 헤더 + 데이터 포함 전체 사이즈를 반환 한다.
		/// @return 에셋 사이즈
		SH_CORE_API auto GetAssetSize() const -> uint64_t;
		/// @brief 에셋의 데이터 사이즈를 반환 한다.
		/// @return 에셋 데이터 사이즈
		SH_CORE_API auto GetAssetDataSize() const -> uint64_t;
		SH_CORE_API auto GetAssetUUID() const -> const UUID&;
		SH_CORE_API auto GetType() const -> const char*;

		/// @brief 마지막 쓰기 시간을 해당 경로에 있는 파일의 마지막 쓰기 시간으로 변경한다. 파일이 없으면 아무 일도 일어나지 않는다.
		/// @param filePath 파일 경로
		SH_CORE_API void SetWriteTime(const std::filesystem::path& filePath);
		/// @brief 마지막 쓰기 시간을 지정한다.
		/// @param time 시간
		SH_CORE_API void SetWriteTime(int64_t time);
		/// @brief 마지막 쓰기 시간을 가져오는 함수.
		/// @return 시간
		SH_CORE_API auto GetWriteTime() const -> int64_t;

		SH_CORE_API auto IsEmpty() const -> bool;

	};
}//namespace