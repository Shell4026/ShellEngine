#include "FileLoader.h"

#include <array>
#include <algorithm>
#include <stdio.h>
#include <string>
namespace sh::core
{
	FileLoader::~FileLoader()
	{

	}

	auto FileLoader::LoadBinary(std::string_view dir) -> std::optional<std::vector<unsigned char>>
	{
		std::vector<unsigned char> data;
		
		FILE* file;
		file = fopen(dir.data(), "rb");
		if (!file)
			return {};

		std::array<unsigned char, 100> buffer{};
		size_t size = 0;
		while ((size = fread(buffer.data(), sizeof(unsigned char), 100, file)) > 0)
		{
			for (int i = 0; i < size; ++i)
				data.push_back(buffer[i]);
		}
		fclose(file);

		return data;
	}
	auto FileLoader::LoadText(std::string_view dir) ->std::optional<std::string>
	{
		std::string strData;

		FILE* file;
		file = fopen(dir.data(), "r");
		if (!file)
			return {};

		std::array<char, 100> buffer{};
		size_t size = 0;
		while ((size = fread(buffer.data(), sizeof(unsigned char), 100, file)) > 0)
		{
			for (int i = 0; i < size; ++i)
				strData.push_back(buffer[i]);
		}
		fclose(file);

		return strData;
	}
}