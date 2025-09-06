message("Build ShellEngine Network")
file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/Network/*.h)

add_library(ShellEngineNetwork SHARED ${SRC} ${HEADERS})
add_library(ShellEngine::Network ALIAS ShellEngineNetwork)

if(WIN32)
set_target_properties(ShellEngineNetwork PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_compile_definitions(ShellEngineNetwork PRIVATE _WIN32_WINNT=0x0601)
elseif(UNIX)
set_target_properties(ShellEngineNetwork PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
	LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")
endif()

include(FetchContent)
FetchContent_Declare(
    asio_external
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio
    GIT_TAG asio-1-34-2
)
FetchContent_Populate(asio_external)

find_package(Threads REQUIRED)
target_link_libraries(ShellEngineNetwork PRIVATE Threads::Threads)
target_link_libraries(ShellEngineNetwork PUBLIC ShellEngine::Core)

target_precompile_headers(ShellEngineNetwork PRIVATE ${CMAKE_SOURCE_DIR}/include/Network/PCH.h)

target_include_directories(ShellEngineNetwork PUBLIC ${asio_external_SOURCE_DIR}/asio/include)
target_include_directories(ShellEngineNetwork PRIVATE ${CMAKE_SOURCE_DIR}/include/Network)
target_include_directories(ShellEngineNetwork PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_include_directories(ShellEngineNetwork PRIVATE ${CMAKE_SOURCE_DIR}/include/external)