#include "ExecuteProcess.h"
#include "Logger.h"

#if _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

namespace sh::core
{
    SH_CORE_API auto ExecuteProcess::Execute(const std::filesystem::path& exe, const std::vector<std::string>& args, std::string& output) -> bool
	{
#if _WIN32
        std::wstring command;
        for (const std::string& arg : args)
        {
            int size = MultiByteToWideChar(CP_UTF8, 0, arg.c_str(), -1, nullptr, 0);
            std::wstring wstr(size, 0);
            MultiByteToWideChar(CP_UTF8, 0, arg.c_str(), -1, &wstr[0], size);
            wstr.pop_back();

            command += wstr;
            command += L" ";
        }

        // 파이프 핸들
        HANDLE hReadPipe = NULL;
        HANDLE hWritePipe = NULL;

        SECURITY_ATTRIBUTES saAttr{};
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        // 출력 파이프 생성
        if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
        {
            SH_ERROR_FORMAT("Stdout pipe creation failed ({})!", GetLastError());
            return false;
        }

        // 부모 프로세스가 파이프에 쓰기 핸들을 상속하지 않도록 설정
        if (!SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0))
        {
            SH_ERROR_FORMAT("Stdout SetHandleInformation failed ({})!", GetLastError());
            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);
            return false;
        }

		STARTUPINFOW si{};
		PROCESS_INFORMATION pi{};

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
        si.hStdOutput = hWritePipe;
        si.hStdError = hWritePipe;
        si.dwFlags |= STARTF_USESTDHANDLES;
		ZeroMemory(&pi, sizeof(pi));

        std::wstring cmdLine = exe.wstring() + L" " + command;
        wchar_t* cmd = const_cast<wchar_t*>(cmdLine.c_str());

        if (!CreateProcessW(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
        {
            SH_ERROR_FORMAT("CreateProcess failed ({})!", GetLastError());
            return false;
        }
        CloseHandle(hWritePipe);

        WaitForSingleObject(pi.hProcess, INFINITE);

        // 출력을 파이프에서 가져옴
        CHAR buffer[4096];
        DWORD bytesRead;
        while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            output += buffer;
        }

        DWORD exitCode;
        if (!GetExitCodeProcess(pi.hProcess, &exitCode)) 
        {
            SH_ERROR_FORMAT("GetExitCodeProcess failed ({})!", GetLastError());
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return false;
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return exitCode == 0;
#elif __linux__
        std::string command;
        for (auto& arg : args)
        {
            command += arg;
            command += " ";
        }

        int pipefd[2]; // 0이 읽기 1이 쓰기 파이프
        if (pipe(pipefd) == -1)
        {
            SH_ERROR_FORMAT("pipe failed: {}", strerror(errno));
            return false;
        }

        pid_t pid = fork(); // Copy-On-Write (COW)기법으로 인해 부하가 그리 크지 않음?
        if (pid == -1) 
        {
            SH_ERROR_FORMAT("fork failed: {}", strerror(errno));
            close(pipefd[0]);
            close(pipefd[1]);
            return false;
        }
        else if (pid == 0) // 자식 프로세스
        {
            // 파이프의 쓰기를 표준 출력과 표준 에러로 설정
            if (dup2(pipefd[1], STDOUT_FILENO) == -1)
            {
                SH_ERROR_FORMAT("dup2 stdout failed: {}", strerror(errno));
                _exit(EXIT_FAILURE);
            }
            if (dup2(pipefd[1], STDERR_FILENO) == -1)
            {
                SH_ERROR_FORMAT("dup2 stderr failed: {}", strerror(errno));
                _exit(EXIT_FAILURE);
            }
            close(pipefd[0]);
            close(pipefd[1]);

            std::vector<char*> execArgs;
            execArgs.push_back(const_cast<char*>(exe.c_str()));
            for (auto& arg : args)
                execArgs.push_back(const_cast<char*>(arg.c_str()));
            execArgs.push_back(nullptr);

            if (execvp(exe.c_str(), execArgs.data()) == -1)
            {
                SH_ERROR_FORMAT("execvp failed: {}", strerror(errno));
                _exit(EXIT_FAILURE);
            }
        }
        else // 부모 프로세스: 자식 프로세스가 종료될 때까지 대기
        { 
            close(pipefd[1]);
            // 출력 읽기
            char buffer[4096];
            ssize_t bytesRead;
            while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
            {
                buffer[bytesRead] = '\0';
                output += buffer;
            }
            if (bytesRead == -1)
            {
                SH_ERROR_FORMAT("read failed: {}", strerror(errno));
                close(pipefd[0]);
                return false;
            }
            close(pipefd[0]);

            int status;
            if (waitpid(pid, &status, 0) == -1) 
            {
                SH_ERROR_FORMAT("waitpid failed: {}", strerror(errno));
                return false;
            }
            if (WIFEXITED(status)) 
            {
                return WEXITSTATUS(status) == 0;
            }
            else // 비정상 종료
            {
                return false;
            }
        }
#endif
        return true;
	}
}//namespace