@echo off
REM 입력된 파일이 있는지 확인
if "%~1"=="" (
    echo 사용법: 이 스크립트에 셰이더 파일을 드래그 앤 드롭하세요.
    pause
    exit /b
)

REM 입력 파일 경로 설정
set "input=%~1"

REM 출력 파일 경로 및 이름 설정 (.spv 확장자)
set "output=%~dpn1%~x1.spv"

REM glslc.exe 경로 설정 (Vulkan SDK 설치 경로에 맞게 수정)
set "glslc=C:\VulkanSDK\1.3.280.0\Bin\glslc.exe"

REM glslc.exe가 존재하는지 확인
if not exist "%glslc%" (
    echo glslc.exe를 찾을 수 없습니다: %glslc%
    pause
    exit /b
)

REM 컴파일 명령 실행
"%glslc%" "%input%" -o "%output%"

REM 컴파일 결과 메시지 출력
if %ERRORLEVEL% EQU 0 (
    echo 컴파일 성공: %output%
) else (
    echo 컴파일 실패
)

pause
