﻿message(STATUS "[ShellEngine] Test Init")

file(GLOB ${hpp} ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

# Google Test
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY "https://github.com/google/googletest"
  GIT_TAG "v1.15.0"
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(ShellEngineTest test.cpp ${hpp})
target_link_libraries(ShellEngineTest gtest gtest_main)
target_link_libraries(ShellEngineTest ShellEngine::Core)
target_link_libraries(ShellEngineTest ShellEngine::Render)
target_link_libraries(ShellEngineTest ShellEngine::Game)

target_include_directories(ShellEngineTest PRIVATE ${CMAKE_SOURCE_DIR}/include)