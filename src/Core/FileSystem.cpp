#include "PCH.h"
#include "FileSystem.h"

#include <utility>
#include <filesystem>
#if WIN32
#include <Windows.h>
#include <shlobj_core.h>
#endif

namespace sh::core
{
	SH_CORE_API auto FileSystem::GetDesktopDirectory() -> std::string
	{
		std::string result{};
#if WIN32
		char buf[MAX_PATH];
		SHGetSpecialFolderPathA(nullptr, buf, CSIDL_DESKTOP, 0);
		result = buf;
#elif __linux
		result = GetHomeDirectory();
#endif
		return result;
	}
	SH_CORE_API auto FileSystem::GetHomeDirectory() -> std::string
	{
#if WIN32
		std::string homeDrive = std::getenv("HOMEDRIVE");
		std::string homePath = std::getenv("HOMEPATH");
		if (!homeDrive.empty() && !homePath.empty())
		{
			return homeDrive + homePath;
		}
#elif __linux__
		std::string homePath = std::getenv("HOME");
		if (!homePath.empty())
		{
			return homePath;
		}
#endif
		return std::filesystem::current_path().root_directory().string();
	}

	SH_CORE_API void FileSystem::CreateFolder(const std::filesystem::path& path, std::string_view folderName)
	{
		std::string name{ folderName };
		int num = 0;
		while (!std::filesystem::create_directory(path / name))
			name = std::string{ folderName } + std::to_string(num++);
	}

	SH_CORE_API auto FileSystem::CreateUniqueFileName(const std::filesystem::path& path, const std::filesystem::path& _fileName) -> std::string
	{
		std::string fileName{ _fileName.stem().string() };
		std::string extension{ _fileName.extension().string() };
		std::string newFileName{ fileName };
		std::filesystem::path filePath = path / _fileName;

		int idx = 0;
		while (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath))
		{
			newFileName = fileName + std::to_string(idx++);
			filePath = path / (newFileName + extension);
		}
		return newFileName + extension;
	}
}//namespace