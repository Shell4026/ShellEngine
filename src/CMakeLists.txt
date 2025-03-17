set(SH_EDITOR TRUE CACHE BOOL "Is Editor?")

add_executable(ShellEngine main.cpp EngineInit.cpp)

target_include_directories(ShellEngine PUBLIC ${CMAKE_SOURCE_DIR}/include)
#target_precompile_headers(ShellEngine PUBLIC ${CMAKE_SOURCE_DIR}/include/Engine.h)

message(STATUS "[ShellEngine] Compiler: ${CMAKE_CXX_COMPILER_ID}")

# FetchContent 를 사용하기 위해 include.
include(FetchContent)

# fmt 라이브러리를 불러온다.
FetchContent_Declare(
  Fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt"
  GIT_TAG "10.2.1"
  )
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
FetchContent_MakeAvailable(Fmt)


add_subdirectory(Core)
target_link_libraries(ShellEngine PUBLIC ShellEngine::Core)

add_subdirectory(Window)
target_link_libraries(ShellEngine PUBLIC ShellEngine::Window)

add_subdirectory(Render)
target_link_libraries(ShellEngine PUBLIC ShellEngine::Render)

add_subdirectory(Physics)
target_link_libraries(ShellEngine PUBLIC ShellEngine::Physics)

add_subdirectory(Game)
target_link_libraries(ShellEngine PUBLIC ShellEngine::Game)

if(SH_EDITOR)
target_compile_definitions(ShellEngineCore PUBLIC SH_EDITOR)
add_subdirectory(Editor)
target_link_libraries(ShellEngine PUBLIC ShellEngine::Editor)
endif()

file(GLOB_RECURSE spvFiles "${CMAKE_SOURCE_DIR}/resource/shaders/*.shader")

file(COPY ${spvFiles} DESTINATION ${CMAKE_BINARY_DIR}/bin/shaders)
file(
	COPY 
	${CMAKE_SOURCE_DIR}/resource/textures
	${CMAKE_SOURCE_DIR}/resource/model
	${CMAKE_SOURCE_DIR}/resource/fonts
	DESTINATION ${CMAKE_BINARY_DIR}/bin)