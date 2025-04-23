set(ENGINE_DIR "Here is directory")
set(PROJECT_NAME "New Project")
file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)
file(GLOB HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h)

cmake_minimum_required (VERSION 3.8)

project (
 ${PROJECT_NAME}
 VERSION 0.1
 LANGUAGES CXX
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
if(WIN32)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
elseif(UNIX)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_library(ShellEngineUser SHARED ${SRC} ${HEADERS})
add_library(ShellEngine::User ALIAS ShellEngineUser)

target_compile_definitions(ShellEngineUser PUBLIC SH_PROJECT_NAME="${PROJECT_NAME}")

target_include_directories(ShellEngineUser PRIVATE ${ENGINE_DIR}/include)
target_link_directories(ShellEngineUser PRIVATE ${ENGINE_DIR}/lib)

target_link_libraries(ShellEngineUser PRIVATE ShellEngineCore)
target_link_libraries(ShellEngineUser PRIVATE ShellEngineRender)
target_link_libraries(ShellEngineUser PRIVATE ShellEngineGame)

include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt"
  GIT_TAG "10.2.1"
  )
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
FetchContent_MakeAvailable(fmt)
target_link_libraries(ShellEngineUser PRIVATE fmt)

FetchContent_Declare(
  GLM
  GIT_REPOSITORY "https://github.com/g-truc/glm"
  GIT_TAG "1.0.1"
  )
FetchContent_MakeAvailable(GLM)
target_link_libraries(ShellEngineUser PRIVATE glm)

FETCHCONTENT_DECLARE(
	JSON
	GIT_REPOSITORY "https://github.com/nlohmann/json"
	GIT_TAG "v3.11.3"
)
FETCHCONTENT_MAKEAVAILABLE(JSON)
target_link_libraries(ShellEngineUser PRIVATE nlohmann_json)

FetchContent_Declare(
  reactphysics3d
  GIT_REPOSITORY "https://github.com/DanielChappuis/reactphysics3d"
  GIT_TAG "v0.10.0"
  )
FetchContent_Populate(reactphysics3d)
target_include_directories(ShellEngineUser PRIVATE ${reactphysics3d_SOURCE_DIR}/include)