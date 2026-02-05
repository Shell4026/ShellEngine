file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

add_library(${EDITOR_TGT} SHARED ${SRCS} ${HEADERS} ${CMAKE_SOURCE_DIR}/PCH.h)

generate_export_header(${EDITOR_TGT}
    BASE_NAME SH_EDIT
    EXPORT_MACRO_NAME SH_EDIT_API
    EXPORT_FILE_NAME ExportEditor.h
)

set_target_properties(${EDITOR_TGT} PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(${EDITOR_TGT} PRIVATE SH_DEBUG)
endif()
target_compile_definitions(${EDITOR_TGT} PRIVATE SH_PROJECT_NAME="${PROJECT_NAME}")
target_compile_definitions(${EDITOR_TGT} PRIVATE SH_TARGET_NAME="${EDITOR_TGT}")

target_precompile_headers(${EDITOR_TGT} PRIVATE ${CMAKE_SOURCE_DIR}/PCH.h)

target_include_directories(${EDITOR_TGT} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${EDITOR_TGT} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${EDITOR_TGT} PRIVATE ${ENGINE_DIR}/include)
target_include_directories(${EDITOR_TGT} PRIVATE ${CMAKE_SOURCE_DIR}/Source)
target_link_directories(${EDITOR_TGT} PRIVATE ${ENGINE_DIR})
target_link_directories(${EDITOR_TGT} PRIVATE ${ENGINE_DIR}/lib)

target_link_libraries(${EDITOR_TGT} PRIVATE ShellEngineCore)
target_link_libraries(${EDITOR_TGT} PRIVATE ShellEngineRender)
target_link_libraries(${EDITOR_TGT} PRIVATE ShellEngineGame)
target_link_libraries(${EDITOR_TGT} PRIVATE ShellEnginePhysics)
target_link_libraries(${EDITOR_TGT} PRIVATE ShellEngineWindow)
target_link_libraries(${EDITOR_TGT} PRIVATE ShellEngineNetwork)
target_link_libraries(${EDITOR_TGT} PRIVATE ShellEngineEditor)
target_link_libraries(${EDITOR_TGT} PRIVATE IMGUI)
target_link_libraries(${EDITOR_TGT} PUBLIC ${TGT})

target_link_libraries(${EDITOR_TGT} PRIVATE fmt)
target_link_libraries(${EDITOR_TGT} PRIVATE glm)
target_link_libraries(${EDITOR_TGT} PRIVATE nlohmann_json)