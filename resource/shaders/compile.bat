@echo off
REM �Էµ� ������ �ִ��� Ȯ��
if "%~1"=="" (
    echo ����: �� ��ũ��Ʈ�� ���̴� ������ �巡�� �� ����ϼ���.
    pause
    exit /b
)

REM �Է� ���� ��� ����
set "input=%~1"

REM ��� ���� ��� �� �̸� ���� (.spv Ȯ����)
set "output=%~dpn1%~x1.spv"

REM glslc.exe ��� ���� (Vulkan SDK ��ġ ��ο� �°� ����)
set "glslc=C:\VulkanSDK\1.3.280.0\Bin\glslc.exe"

REM glslc.exe�� �����ϴ��� Ȯ��
if not exist "%glslc%" (
    echo glslc.exe�� ã�� �� �����ϴ�: %glslc%
    pause
    exit /b
)

REM ������ ��� ����
"%glslc%" "%input%" -o "%output%"

REM ������ ��� �޽��� ���
if %ERRORLEVEL% EQU 0 (
    echo ������ ����: %output%
) else (
    echo ������ ����
)

pause
