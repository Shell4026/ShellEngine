file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

add_library(${TGT} SHARED ${SRCS} ${HEADERS} ${CMAKE_SOURCE_DIR}/PCH.h)

generate_export_header(${TGT}
    BASE_NAME SH_USER
    EXPORT_MACRO_NAME SH_USER_API
    EXPORT_FILE_NAME Export.h
)

set_target_properties(${TGT} PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(${TGT} PRIVATE SH_DEBUG)
endif()
target_compile_definitions(${TGT} PRIVATE SH_PROJECT_NAME="${PROJECT_NAME}")
target_compile_definitions(${TGT} PRIVATE SH_TARGET_NAME="${TGT}")
target_precompile_headers(${TGT} PRIVATE ${CMAKE_SOURCE_DIR}/PCH.h)

target_include_directories(${TGT} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TGT} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${TGT} PRIVATE ${ENGINE_DIR}/include)
target_link_directories(${TGT} PRIVATE ${ENGINE_DIR})
target_link_directories(${TGT} PRIVATE ${ENGINE_DIR}/lib)

target_link_libraries(${TGT} PRIVATE ShellEngineCore)
target_link_libraries(${TGT} PRIVATE ShellEngineRender)
target_link_libraries(${TGT} PRIVATE ShellEngineGame)
target_link_libraries(${TGT} PRIVATE ShellEnginePhysics)
target_link_libraries(${TGT} PRIVATE ShellEngineWindow)
target_link_libraries(${TGT} PRIVATE ShellEngineNetwork)
target_link_libraries(${TGT} PRIVATE IMGUI)

target_link_libraries(${TGT} PRIVATE fmt)
target_link_libraries(${TGT} PRIVATE glm)
target_link_libraries(${TGT} PRIVATE nlohmann_json)