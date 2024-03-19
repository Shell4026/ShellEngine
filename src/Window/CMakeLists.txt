file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if(WIN32)
	message(STATUS "OS: Window")
	file(GLOB_RECURSE OS_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Win32/*.cpp)
elseif(UNIX)
	message(STATUS "OS: Unix")
	file(GLOB_RECURSE OS_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Unix/*.cpp)
endif()

add_library(ShellEngineWindow STATIC ${SRC} ${OS_SRC})

target_include_directories(ShellEngineWindow PUBLIC ${CMAKE_SOURCE_DIR}/include/Window)