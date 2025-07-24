#pragma once
#include "Export.h"

#include <filesystem>
#include <vector>
#include <string>
namespace sh::core
{
	/// @brief 크로스 플랫폼 프로세스 실행 클래스
	class ExecuteProcess
	{
	public:
		/// @brief 동기적으로 프로세스를 실행한다.
		/// @param exe 실행 파일 경로
		/// @param args 전달 할 인자
		/// @param output 해당 프로세스의 출력을 담는 string
		/// @return 실행 결과
		SH_CORE_API static auto Execute(const std::filesystem::path& exe, const std::vector<std::string>& args, std::string& output) -> bool;
	};
}//namespace