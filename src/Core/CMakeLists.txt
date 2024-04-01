file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(ShellEngineCore STATIC ${SRC})
add_library(ShellEngine::Core ALIAS ShellEngineCore)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(ShellEngineCore PUBLIC SH_DEBUG)
endif()

target_include_directories(ShellEngineCore PUBLIC ${CMAKE_SOURCE_DIR}/include/Core)

target_link_libraries(ShellEngineCore PUBLIC fmt)

