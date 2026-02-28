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

	set_target_properties(ShellEngineWindow PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
elseif(UNIX)
	target_link_libraries(ShellEngineWindow PRIVATE ${X11_LIBRARIES})

	set_target_properties(ShellEngineWindow PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
	LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")
endif()

target_include_directories(ShellEngineWindow PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(ShellEngineWindow PRIVATE ${CMAKE_SOURCE_DIR}/include/Window)

target_link_libraries(ShellEngineWindow PUBLIC ShellEngine::Core)