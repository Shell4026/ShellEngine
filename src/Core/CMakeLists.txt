message("Build ShellEngine Core")
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/Core/*.h)

add_library(ShellEngineCore SHARED ${SRC} ${HEADERS})
add_library(ShellEngine::Core ALIAS ShellEngineCore)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(ShellEngineCore PUBLIC SH_DEBUG)
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

target_precompile_headers(ShellEngineCore PUBLIC ${CMAKE_SOURCE_DIR}/include/Core/PCH.h)

target_include_directories(ShellEngineCore PRIVATE ${CMAKE_SOURCE_DIR}/include/Core)
