#include "Util.h"
#include "SObject.h"

#include <random>
#include <limits>
#include <sstream>
#include <iomanip>
#include <regex>

#ifdef max
#undef max
#endif

namespace sh::core {
	auto Util::U8StringToWstring(const std::string& u8str) -> std::wstring
	{
		std::wstring result;
		for (int i = 0; i < u8str.size();)
		{
			unsigned char c0 = u8str[i];
			if (c0 >> 3 == 0b11110) //4byte utf
			{
				c0 = (c0 & 0b00000111);
				char c1 = (u8str[i + 1] & 0b00111111);
				char c2 = (u8str[i + 2] & 0b00111111);
				char c3 = (u8str[i + 3] & 0b00111111);
				int unicode = (c3 | c2 << 6) | ((c2 >> 2 | c1 << 4) << 8) | ((c1 >> 4 | c0 << 2) << 16);
				//UTF-16 incoding
				wchar_t high = 0xD800 + ((unicode - 0x10000) / 0x400);
				wchar_t low = 0xDC00 + ((unicode - 0x10000) % 0x400);
				result += high;
				result += low;
				i += 4;
			}
			else if (c0 >> 4 == 0b1110) //3byte utf
			{
				c0 = (c0 & 0b00001111);
				char c1 = (u8str[i + 1] & 0b00111111);
				char c2 = (u8str[i + 2] & 0b00111111);
				result += (c2 | c1 << 6) | ((c1 >> 2 | c0 << 4) << 8);
				i += 3;
			}
			else if (c0 >> 5 == 0b110) //2byte utf
			{
				c0 = (c0 & 0b00011111);
				char c1 = (u8str[i + 1] & 0b00111111);
				result += (c1 | c0 << 6) | (c0 >> 2 << 8);
				i += 2;
			}
			else if (c0 >> 7 == 0b0) //1byte utf
			{
				result += c0;
				++i;
			}
		}
		return result;
	}

	bool IsValid(const SObject* obj)
	{
		if (obj == nullptr)
			return false;
		return !obj->IsPendingKill();
	}

	auto Util::AlignTo(uint32_t value, uint32_t alignment) -> uint32_t
	{
		// ex) value = 20, alignment = 16
		// value = 0001'0100
		// alignment = 0001'0000
		// alignment - 1 = 0000'1111
		// ~(alignment - 1) = 1111'0000 - 끝 4비트가 0이면 16의 배수라는 뜻.
		// 35 = 0010'0011
		// 0010'0011 & 1111'0000 = 0010'0000 = 32 
		return (value + alignment - 1) & ~(alignment - 1);
	}

	SH_CORE_API auto Util::RandomRange(uint32_t min, uint32_t max) -> uint32_t
	{
		std::uniform_int_distribution<uint32_t> rnd{ min, max };
		return rnd(gen);
	}
	SH_CORE_API auto Util::RandomRange(int min, int max) -> int
	{
		std::uniform_int_distribution<int> rnd{ min, max };
		return rnd(gen);
	}
	SH_CORE_API auto Util::RandomRange(float min, float max) -> float
	{
		std::uniform_real_distribution<float> rnd{ min, max };
		return rnd(gen);
	}
	SH_CORE_API auto Util::RandomRange(double min, double max) -> double
	{
		std::uniform_real_distribution<double> rnd{ min, max };
		return rnd(gen);
	}

	SH_CORE_API auto Util::ReplaceSpaceString(const std::string& str) -> std::string
	{
		std::string result;
		result.reserve(str.size());
		for (char c : str)
		{
			if (std::isalnum(static_cast<unsigned char>(c)))
				result.push_back(c);
			else
				result.push_back('_');
		}
		return result;
	}

	SH_CORE_API auto Util::ConvertByteToWord(const std::vector<uint8_t>& bytes) -> std::vector<uint32_t>
	{
		std::vector<uint32_t> result;
		uint32_t word = 0;
		for (int i = 0; i < bytes.size(); ++i)
		{
			word |= static_cast<uint32_t>(bytes[i]) << (i % 4) * 8;
			if ((i + 1) % 4 == 0)
			{
				result.push_back(word);
				word = 0;
			}
		}

		if (bytes.size() % 4 != 0)
		{
			result.push_back(word);
		}
		return result;
	}

	SH_CORE_API auto Util::ConvertMat2ToMat4(const glm::mat2& mat) -> glm::mat4
	{
		return glm::mat4
		{
			mat[0][0], mat[0][1], 0.f, 0.f,
			mat[1][0], mat[1][1], 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f
		};
	}
	SH_CORE_API auto Util::ConvertMat4ToMat2(const glm::mat4& mat) -> glm::mat2
	{
		return glm::mat2
		{
			mat[0][0], mat[0][1],
			mat[1][0], mat[1][1],
		};
	}
	SH_CORE_API auto Util::ConvertMat3ToMat4(const glm::mat3& mat) -> glm::mat4
	{
		return glm::mat4
		{
			mat[0][0], mat[0][1], mat[0][2], 0.f,
			mat[1][0], mat[1][1], mat[1][2], 0.f,
			mat[2][0], mat[2][1], mat[2][2], 0.f,
			0.f, 0.f, 0.f, 0.f
		};
	}
	SH_CORE_API auto Util::ConvertMat4ToMat3(const glm::mat4& mat) -> glm::mat3
	{
		return glm::mat3
		{
			mat[0][0], mat[0][1], mat[0][2],
			mat[1][0], mat[1][1], mat[1][2],
			mat[2][0], mat[2][1], mat[2][2]
		};
	}
	SH_CORE_API auto Util::ExtractUUIDs(const core::Json& json) -> std::vector<std::string>
	{
		std::unordered_set<std::string> uuids;
		ExtractUUIDsHelper(uuids, json);

		return std::vector<std::string>{ uuids.begin(), uuids.end() };
	}
	void Util::ExtractUUIDsHelper(std::unordered_set<std::string>& uuids, const core::Json& json)
	{
		static std::regex uuidRegex{ "^[0-9a-f]{32}$", std::regex::optimize };
		if (json.is_object())
		{
			for (auto const& [key, val] : json.items())
			{
				ExtractUUIDsHelper(uuids, val);
			}
		}
		else if (json.is_array())
		{
			for (const auto& item : json)
			{
				ExtractUUIDsHelper(uuids, item);
			}
		}
		else if (json.is_string())
		{
			const std::string& value = json.get<std::string>();
			if (std::regex_match(value, uuidRegex))
				uuids.insert(value);
		}
	}
}