set(SH_EDITOR TRUE CACHE BOOL "Is Editor?")

if (SH_EDITOR)
set(EXE_NAME ShellEngine)
else()
set(EXE_NAME ShellGame)
endif()

add_executable(${EXE_NAME} main.cpp EngineInit.cpp)

set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_include_directories(${EXE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
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
target_link_libraries(${EXE_NAME} PUBLIC ShellEngine::Core)

add_subdirectory(Window)
target_link_libraries(${EXE_NAME} PUBLIC ShellEngine::Window)

add_subdirectory(Render)
target_link_libraries(${EXE_NAME} PUBLIC ShellEngine::Render)

add_subdirectory(Physics)
target_link_libraries(${EXE_NAME} PUBLIC ShellEngine::Physics)

add_subdirectory(Game)
target_link_libraries(${EXE_NAME} PUBLIC ShellEngine::Game)

add_subdirectory(Network)
target_link_libraries(${EXE_NAME} PUBLIC ShellEngine::Network)

if(SH_EDITOR)
target_compile_definitions(ShellEngineCore PUBLIC SH_EDITOR)
add_subdirectory(Editor)
target_link_libraries(${EXE_NAME} PUBLIC ShellEngine::Editor)

add_custom_target(EngineTarget ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/bin/include
)

file(COPY ${CMAKE_SOURCE_DIR}/resource/ProjectTemplate DESTINATION ${CMAKE_BINARY_DIR}/bin)
endif()

file(GLOB_RECURSE spvFiles "${CMAKE_SOURCE_DIR}/resource/shaders/*.shader")

file(COPY ${spvFiles} DESTINATION ${CMAKE_BINARY_DIR}/bin/shaders)
file(
	COPY 
	${CMAKE_SOURCE_DIR}/resource/textures
	${CMAKE_SOURCE_DIR}/resource/model
	${CMAKE_SOURCE_DIR}/resource/fonts
	DESTINATION ${CMAKE_BINARY_DIR}/bin)