file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(ShellEngineCore STATIC ${SRC})
add_library(ShellEngine::Core ALIAS ShellEngineCore)

target_include_directories(ShellEngineCore PUBLIC ${CMAKE_SOURCE_DIR}/include/Core)