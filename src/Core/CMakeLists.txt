message("Build ShellEngine Core")
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/Core/*.h ${CMAKE_SOURCE_DIR}/include/Core/*.hpp)
set(LZ4_SRC ${CMAKE_SOURCE_DIR}/include/External/lz4/lz4.cpp)

add_library(ShellEngineCore SHARED ${SRC} ${LZ4_SRC} ${HEADERS})
add_library(ShellEngine::Core ALIAS ShellEngineCore)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(ShellEngineCore PUBLIC SH_DEBUG)
endif()

if(WIN32)
set_target_properties(ShellEngineCore PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
elseif(UNIX)
set_target_properties(ShellEngineCore PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
	LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")
endif()

FetchContent_Declare(
  GLM
  GIT_REPOSITORY "https://github.com/g-truc/glm"
  GIT_TAG "1.0.1"
  )
FetchContent_MakeAvailable(GLM)

FETCHCONTENT_DECLARE(
	JSON
	GIT_REPOSITORY "https://github.com/nlohmann/json"
	GIT_TAG "v3.11.3"
)
FETCHCONTENT_MAKEAVAILABLE(JSON)

target_link_libraries(ShellEngineCore PUBLIC glm)
target_link_libraries(ShellEngineCore PUBLIC fmt)
target_link_libraries(ShellEngineCore PUBLIC nlohmann_json)

target_precompile_headers(ShellEngineCore PRIVATE ${CMAKE_SOURCE_DIR}/include/Core/PCH.h)

target_include_directories(ShellEngineCore PRIVATE ${CMAKE_SOURCE_DIR}/include/Core)