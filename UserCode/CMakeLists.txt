set(SH_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include" CACHE STRINGS "Engine include directory")

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(ShellEngineUser SHARED ${SRC})
add_library(ShellEngine::User ALIAS ShellEngineUser)

target_link_libraries(ShellEngineUser PRIVATE ShellEngine::Core)
target_link_libraries(ShellEngineUser PRIVATE ShellEngine::Game)

target_include_directories(ShellEngineUser PRIVATE ${SH_INCLUDE_DIR})