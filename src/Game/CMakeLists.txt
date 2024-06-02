message("Build ShellEngine Game")
file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB COMPONENT_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Component/*.cpp)

add_library(ShellEngineGame SHARED ${SRC} ${COMPONENT_SRC})
add_library(ShellEngine::Game ALIAS ShellEngineGame)

target_link_libraries(ShellEngineGame PUBLIC ShellEngine::Core)
target_link_libraries(ShellEngineGame PUBLIC ShellEngine::Window)
target_link_libraries(ShellEngineGame PUBLIC ShellEngine::Render)
target_link_libraries(ShellEngineGame PUBLIC IMGUI)

FetchContent_Declare(
  reactphysics3d
  GIT_REPOSITORY "https://github.com/DanielChappuis/reactphysics3d"
  GIT_TAG "v0.10.0"
  )
FetchContent_MakeAvailable(reactphysics3d)
target_link_libraries(ShellEngineGame PUBLIC reactphysics3d)

target_include_directories(ShellEngineGame PRIVATE ${CMAKE_SOURCE_DIR}/include/Game)
target_include_directories(ShellEngineGame PRIVATE ${CMAKE_SOURCE_DIR}/include/external)
target_include_directories(ShellEngineGame PRIVATE ${REACTPHYSICS3D_HEADERS})
