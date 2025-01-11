#pragma once
#include "Export.h"

#include <string>
#include <filesystem>
#include <optional>

namespace sh::core
{
	class FileSystem
	{
	public:
		SH_CORE_API static auto GetDesktopDirectory() -> std::string;
		SH_CORE_API static auto GetHomeDirectory() -> std::string;
		/// @brief 해당 경로에 폴더를 만드는 함수. 이미 존재한다면 뒤에 숫자를 붙인다.
		/// @param path 경로
		/// @param name 폴더 이름
		SH_CORE_API static void CreateFolder(const std::filesystem::path& path, std::string_view folderName);
		/// @brief 해당 경로에 같은 이름의 파일이 존재 할 시 이름에 숫자를 붙여 반환하는 함수
		/// @param path 경로
		/// @param fileName 파일 이름
		/// @return 새 파일 이름
		SH_CORE_API static auto CreateUniqueFileName(const std::filesystem::path& path, const std::filesystem::path& fileName) -> std::string;

		SH_CORE_API static auto LoadBinary(const std::filesystem::path& path) -> std::optional<std::vector<unsigned char>>;
		SH_CORE_API static auto SaveBinary(const std::vector<uint8_t>& binary, const std::filesystem::path& path) -> bool;
		SH_CORE_API static auto LoadText(const std::filesystem::path& path) -> std::optional<std::string>;
		SH_CORE_API static auto SaveText(const std::string& text, const std::filesystem::path& path) -> bool;
	};
}//namespace