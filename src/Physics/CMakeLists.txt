message("Build ShellEngine Physics")

file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/*.h ${CMAKE_SOURCE_DIR}/include/*.hpp)

add_library(ShellEnginePhysics SHARED ${SRC} ${HEADERS})
add_library(ShellEngine::Physics ALIAS ShellEnginePhysics)

if(WIN32)
set_target_properties(ShellEnginePhysics PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
elseif(UNIX)
set_target_properties(ShellEnginePhysics PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
	LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")
endif()

target_link_libraries(ShellEnginePhysics PUBLIC ShellEngine::Core)

FetchContent_Declare(
  reactphysics3d
  GIT_REPOSITORY "https://github.com/DanielChappuis/reactphysics3d"
  GIT_TAG "v0.10.2"
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