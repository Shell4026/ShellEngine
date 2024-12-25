message("Build ShellEngine Window")
file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS${CMAKE_SOURCE_DIR}/include/Window/*.h)

if(WIN32)
	message(STATUS "OS: Window")
	file(GLOB_RECURSE OS_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Win32/*.cpp)
elseif(UNIX)
	message(STATUS "OS: Unix")

	find_package(X11 REQUIRED)
	if(NOT X11_FOUND)
		message(FATAL_ERROR "Could not found X11")
	endif()
	file(GLOB_RECURSE OS_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Unix/*.cpp)
endif()

add_library(ShellEngineWindow SHARED ${SRC} ${OS_SRC})
add_library(ShellEngine::Window ALIAS ShellEngineWindow)

if(WIN32)
	target_link_libraries(ShellEngineWindow PRIVATE Winmm)
elseif(UNIX)
	target_link_libraries(ShellEngineWindow PRIVATE ${X11_LIBRARIES})
endif()

target_include_directories(ShellEngineWindow PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(ShellEngineWindow PRIVATE ${CMAKE_SOURCE_DIR}/include/Window)

target_link_libraries(ShellEngineWindow PUBLIC ShellEngine::Core)

#GLEW를 불러온다
FetchContent_Declare(
  GLEW
  GIT_REPOSITORY https://github.com/Perlmint/glew-cmake
  GIT_TAG        glew-cmake-2.2.0
  )
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
FetchContent_MakeAvailable(GLEW)


#GLEW 라이브러리 출력 파일 위치 지정
set_target_properties(libglew_shared PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set_target_properties(libglew_shared PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set_target_properties(libglew_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

target_link_libraries(ShellEngineWindow PUBLIC libglew_shared)