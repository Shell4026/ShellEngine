message("Build ShellEngine Game")
file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB COMPONENT_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Component/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/Game/*.h)

add_library(ShellEngineGame SHARED ${SRC} ${COMPONENT_SRC} ${HEADERS})
add_library(ShellEngine::Game ALIAS ShellEngineGame)

if(WIN32)
set_target_properties(ShellEngineGame PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
elseif(UNIX)
set_target_properties(ShellEngineGame PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
	LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")
endif()

target_link_libraries(ShellEngineGame PUBLIC ShellEngine::Core)
target_link_libraries(ShellEngineGame PUBLIC ShellEngine::Window)
target_link_libraries(ShellEngineGame PUBLIC ShellEngine::Render)
target_link_libraries(ShellEngineGame PUBLIC ShellEngine::Physics)
target_link_libraries(ShellEngineGame PUBLIC ShellEngine::Network)
target_link_libraries(ShellEngineGame PUBLIC IMGUI)

target_precompile_headers(ShellEngineGame PRIVATE ${CMAKE_SOURCE_DIR}/include/Game/PCH.h)

target_include_directories(ShellEngineGame PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(ShellEngineGame PRIVATE ${CMAKE_SOURCE_DIR}/include/Game)
target_include_directories(ShellEngineGame PRIVATE ${CMAKE_SOURCE_DIR}/include/external)