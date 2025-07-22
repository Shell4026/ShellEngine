#include "FileSystem.h"
#include "Logger.h"

#include <utility>
#include <filesystem>
#if WIN32
#include <Windows.h>
#include <shlobj_core.h>
#endif

#include <array>
#include <cstdint>

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
	SH_CORE_API void FileSystem::CopyAllFiles(const std::filesystem::path& source, const std::filesystem::path& target)
	{
		for (const auto& entry : std::filesystem::directory_iterator(source)) 
		{
			const auto& path = entry.path();
			auto dstPath = target / path.filename();

			if (std::filesystem::is_directory(path)) 
			{
				std::filesystem::create_directories(dstPath);
				CopyAllFiles(path, dstPath);
			}
			else if (std::filesystem::is_regular_file(path)) 
			{
				std::filesystem::copy_file(path, dstPath, std::filesystem::copy_options::overwrite_existing);
			}
		}
	}
	SH_CORE_API auto FileSystem::LoadBinary(const std::filesystem::path& path) -> std::optional<std::vector<unsigned char>>
	{
		FILE* file = nullptr;
#if WIN32
		file = _wfopen(path.c_str(), L"rb");
#else
		file = fopen(path.c_str(), "rb");
#endif
		if (file == nullptr)
			return {};

		std::vector<uint8_t> data(std::filesystem::file_size(path), 0);
		if (data.size() == 0)
		{
			fclose(file);
			return data;
		}

		if (std::size_t size = fread(data.data(), sizeof(uint8_t), data.size(), file); size != data.size())
		{
			SH_ERROR_FORMAT("Loaded file size is {} (expected: {})", size, data.size());
			fclose(file);
			return std::nullopt;
		}
		fclose(file);

		return data;
	}
	SH_CORE_API auto FileSystem::SaveBinary(const std::vector<uint8_t>& binary, const std::filesystem::path& path) -> bool
	{
		FILE* file;
#if WIN32
		file = _wfopen(path.c_str(), L"wb");
#else
		file = fopen(path.c_str(), "wb");
#endif
		if (file == nullptr)
			return false;

		fwrite(binary.data(), sizeof(uint8_t), binary.size(), file);
		fclose(file);

		return true;
	}
	SH_CORE_API auto FileSystem::LoadText(const std::filesystem::path& path) -> std::optional<std::string>
	{
		std::string strData;

		FILE* file = nullptr;
#if WIN32
		file = _wfopen(path.c_str(), L"rb");
#else
		file = fopen(path.c_str(), "rb");
#endif
		if (file == nullptr)
			return {};

		auto fileSize = std::filesystem::file_size(path);
		strData.resize(fileSize);

		fread(strData.data(), sizeof(unsigned char), fileSize, file);
		fclose(file);

		return strData;
	}
	SH_CORE_API auto FileSystem::SaveText(const std::string& text, const std::filesystem::path& path) -> bool
	{
		FILE* file;
#if WIN32
		file = _wfopen(path.c_str(), L"wb");
#else
		file = fopen(path.c_str(), "wb");
#endif
		if (file == nullptr)
			return false;

		fwrite(text.data(), sizeof(char), text.size(), file);
		fclose(file);

		return true;
	}
}//namespace