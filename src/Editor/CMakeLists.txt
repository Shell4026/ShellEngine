message("Build ShellEngine Editor")

file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB COMPONENT_SOURCE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Component/*.cpp)
file(GLOB UI_SOURCE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/UI/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/Editor/*.h)

add_library(ShellEngineEditor SHARED ${SRC} ${COMPONENT_SOURCE} ${UI_SOURCE} ${HEADERS})
add_library(ShellEngine::Editor ALIAS ShellEngineEditor)

target_link_libraries(ShellEngineEditor PUBLIC ShellEngine::Core)
target_link_libraries(ShellEngineEditor PUBLIC ShellEngine::Window)
target_link_libraries(ShellEngineEditor PUBLIC ShellEngine::Render)
target_link_libraries(ShellEngineEditor PUBLIC ShellEngine::Game)

target_precompile_headers(ShellEngineEditor PRIVATE ${CMAKE_SOURCE_DIR}/include/Game/PCH.h)

target_include_directories(ShellEngineEditor PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_include_directories(ShellEngineEditor PUBLIC ${CMAKE_SOURCE_DIR}/include/Editor)