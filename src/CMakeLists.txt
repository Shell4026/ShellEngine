add_executable(ShellEngine main.cpp "Render/VulkanRenderer.cpp")

target_include_directories(ShellEngine PUBLIC ${CMAKE_SOURCE_DIR}/include)
#target_precompile_headers(ShellEngine PUBLIC ${CMAKE_SOURCE_DIR}/include/Engine.h)

# FetchContent 를 사용하기 위해 include.
include(FetchContent)

# fmt 라이브러리를 불러온다.
FetchContent_Declare(
  Fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt"
  GIT_TAG "10.2.1"
  )
FetchContent_MakeAvailable(Fmt)

target_link_libraries(ShellEngine PUBLIC fmt)

add_subdirectory(Core)
target_link_libraries(ShellEngine PUBLIC ShellEngineCore)

add_subdirectory(Window)
target_link_libraries(ShellEngine PUBLIC ShellEngineWindow)

add_subdirectory(Render)
target_link_libraries(ShellEngine PUBLIC ShellEngineRender)