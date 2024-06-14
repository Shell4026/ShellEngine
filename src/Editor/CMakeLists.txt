message("Build ShellEngine Editor")

file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(ShellEngineEditor SHARED ${SRC})
add_library(ShellEngine::Editor ALIAS ShellEngineEditor)

target_link_libraries(ShellEngineEditor PUBLIC ShellEngine::Core)
target_link_libraries(ShellEngineEditor PUBLIC ShellEngine::Window)
target_link_libraries(ShellEngineEditor PUBLIC ShellEngine::Render)
target_link_libraries(ShellEngineEditor PUBLIC ShellEngine::Game)

target_include_directories(ShellEngineEditor PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_include_directories(ShellEngineEditor PUBLIC ${CMAKE_SOURCE_DIR}/include/Editor/)