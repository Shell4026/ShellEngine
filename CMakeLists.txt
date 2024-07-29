# CMakeList.txt : 최상위 CMake 프로젝트 파일, 전역 구성을 수행하고
# 여기에 하위 프로젝트를 포함합니다.
#
cmake_minimum_required (VERSION 3.8)

project (
 ShellEngineProject
 VERSION 0.1
 LANGUAGES CXX)

 set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
 set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

 set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

 set(CMAKE_CXX_STANDARD 17)
# 하위 프로젝트를 포함합니다.
add_subdirectory (src)
add_subdirectory (include/External/imgui)
add_subdirectory (UserCode)
add_subdirectory (Test)