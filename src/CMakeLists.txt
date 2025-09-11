set(EDITOR ShellEngine)
set(GAME ShellGame)
set(TARGETS ${EDITOR} ${GAME})

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
add_subdirectory(Window)
add_subdirectory(Render)
add_subdirectory(Physics)
add_subdirectory(Game)
add_subdirectory(Network)

foreach(TGT ${TARGETS})
	add_executable(${TGT} main.cpp EngineInit.cpp)
	set_target_properties(${TGT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
	target_include_directories(${TGT} PUBLIC ${CMAKE_SOURCE_DIR}/include)

	target_link_libraries(${TGT} PUBLIC ShellEngine::Core)
	target_link_libraries(${TGT} PUBLIC ShellEngine::Window)
	target_link_libraries(${TGT} PUBLIC ShellEngine::Render)
	target_link_libraries(${TGT} PUBLIC ShellEngine::Physics)
	target_link_libraries(${TGT} PUBLIC ShellEngine::Game)
	target_link_libraries(${TGT} PUBLIC ShellEngine::Network)

	if("${TGT}" STREQUAL "${EDITOR}")
		target_compile_definitions(${TGT} PUBLIC SH_EDITOR)
		add_subdirectory(Editor)
		target_link_libraries(${TGT} PUBLIC ShellEngine::Editor)

		add_custom_target(EngineTarget ALL
			COMMAND ${CMAKE_COMMAND} -E copy_directory 
				${CMAKE_SOURCE_DIR}/include
				${CMAKE_BINARY_DIR}/bin/include
		)
		file(COPY ${CMAKE_SOURCE_DIR}/resource/ProjectTemplate DESTINATION ${CMAKE_BINARY_DIR}/bin)
	endif()
endforeach()

file(GLOB_RECURSE spvFiles "${CMAKE_SOURCE_DIR}/resource/shaders/*.shader")

file(COPY ${spvFiles} DESTINATION ${CMAKE_BINARY_DIR}/bin/shaders)
file(
	COPY 
	${CMAKE_SOURCE_DIR}/resource/textures
	${CMAKE_SOURCE_DIR}/resource/model
	${CMAKE_SOURCE_DIR}/resource/fonts
	DESTINATION ${CMAKE_BINARY_DIR}/bin)