set(SRC "imgui.cpp" "imgui_demo.cpp" "imgui_draw.cpp" "imgui_tables.cpp" "imgui_widgets.cpp"
"backends/imgui_impl_vulkan.cpp"
"backends/imgui_impl_win32.cpp")

add_library(IMGUI STATIC ${SRC})

target_link_libraries(IMGUI PUBLIC ShellEngineRender)
target_include_directories(IMGUI PUBLIC ${CMAKE_CURRENT_LIST_DIR})

message(${CMAKE_CURRENT_LIST_DIR})
