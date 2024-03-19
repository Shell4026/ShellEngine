add_library(ShellEngineWindow STATIC Window.cpp)

target_include_directories(ShellEngineWindow PUBLIC ${CMAKE_SOURCE_DIR}/include/Window)