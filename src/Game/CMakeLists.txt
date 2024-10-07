message("Build ShellEngine Game")
file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB COMPONENT_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Component/*.cpp)

add_library(ShellEngineGame SHARED ${SRC} ${COMPONENT_SRC})
add_library(ShellEngine::Game ALIAS ShellEngineGame)

target_link_libraries(ShellEngineGame PUBLIC ShellEngine::Core)
target_link_libraries(ShellEngineGame PUBLIC ShellEngine::Window)
target_link_libraries(ShellEngineGame PUBLIC ShellEngine::Render)
target_link_libraries(ShellEngineGame PUBLIC ShellEngine::Physics)
target_link_libraries(ShellEngineGame PUBLIC IMGUI)

target_precompile_headers(ShellEngineGame PUBLIC ${CMAKE_SOURCE_DIR}/include/Game/PCH.h)

target_include_directories(ShellEngineGame PRIVATE ${CMAKE_SOURCE_DIR}/include/Game)
target_include_directories(ShellEngineGame PRIVATE ${CMAKE_SOURCE_DIR}/include/external)