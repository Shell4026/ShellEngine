message("Build ShellEngine Physics")

file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(ShellEnginePhysics SHARED ${SRC})
add_library(ShellEngine::Physics ALIAS ShellEnginePhysics)

target_link_libraries(ShellEnginePhysics PUBLIC ShellEngine::Core)

FetchContent_Declare(
  reactphysics3d
  GIT_REPOSITORY "https://github.com/DanielChappuis/reactphysics3d"
  GIT_TAG "v0.10.0"
  )
FetchContent_MakeAvailable(reactphysics3d)
target_link_libraries(ShellEnginePhysics PUBLIC reactphysics3d)

FetchContent_Declare(
  GLM
  GIT_REPOSITORY "https://github.com/g-truc/glm"
  GIT_TAG "1.0.1"
  )
FetchContent_MakeAvailable(GLM)
target_link_libraries(ShellEnginePhysics PUBLIC glm)

target_precompile_headers(ShellEnginePhysics PUBLIC ${CMAKE_SOURCE_DIR}/include/Physics/PCH.h)

target_include_directories(ShellEnginePhysics PUBLIC ${CMAKE_SOURCE_DIR}/include/Physics)
target_include_directories(ShellEnginePhysics PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(ShellEnginePhysics PUBLIC ${REACTPHYSICS3D_HEADERS})