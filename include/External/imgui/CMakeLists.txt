set(SRC "imgui.cpp" "imgui_demo.cpp" "imgui_draw.cpp" "imgui_tables.cpp" "imgui_widgets.cpp" "ImGuizmo.cpp"
"backends/imgui_impl_vulkan.cpp" "misc/cpp/imgui_stdlib.cpp"
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(IMGUI STATIC ${SRC})

find_package(Vulkan)
target_link_libraries(IMGUI PRIVATE ShellEngine::Render)

target_include_directories(IMGUI PUBLIC ${CMAKE_CURRENT_LIST_DIR})

message(${CMAKE_CURRENT_LIST_DIR})
