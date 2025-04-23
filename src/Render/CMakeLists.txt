message("Build ShellEngine Renderer")
file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Mesh/*.cpp)
file(GLOB HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/Render/*.{h,hpp})
file(GLOB VULKAN_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/VulkanImpl/*.cpp)
file(GLOB VULKAN_HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/Render/VulkanImpl/*.{h,hpp})

add_library(ShellEngineRender SHARED ${SRC} ${HEADERS} ${VULKAN_SRC} ${VULKAN_HEADERS})
add_library(ShellEngine::Render ALIAS ShellEngineRender)

if(WIN32)
set_target_properties(ShellEngineRender PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
elseif(UNIX)
set_target_properties(ShellEngineRender PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
	LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")
endif()

target_link_libraries(ShellEngineRender PUBLIC ShellEngine::Core)
target_link_libraries(ShellEngineRender PUBLIC ShellEngine::Window)

find_package(Vulkan)

if (NOT Vulkan_FOUND)
	message("Vulkan not found. Search using the manual path...")
	if (WIN32)
		set(VULKAN_SDK_DIR "C:/VulkanSDK/1.4.309.0")
		set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_DIR}/Include")
		set(Vulkan_LIBRARY_DIR "${VULKAN_SDK_DIR}/Lib")
	endif()
endif()

find_package(Vulkan REQUIRED COMPONENTS glslc glslangValidator)
file(COPY ${Vulkan_GLSLC_EXECUTABLE} DESTINATION ${CMAKE_BINARY_DIR}/bin)

target_link_libraries(ShellEngineRender PUBLIC Vulkan::Vulkan)

include(FetchContent)
FetchContent_Declare(
  VMA
  GIT_REPOSITORY "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator"
  GIT_TAG "v3.0.1"
)
FetchContent_MakeAvailable(VMA)
target_link_libraries(ShellEngineRender PRIVATE VulkanMemoryAllocator)

target_precompile_headers(ShellEngineRender PUBLIC "${CMAKE_SOURCE_DIR}/include/Render/pch.h")

target_include_directories(ShellEngineRender PUBLIC ${Vulkan_INCLUDE_DIR})
target_include_directories(ShellEngineRender PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(ShellEngineRender PRIVATE ${CMAKE_SOURCE_DIR}/include/Render)
target_include_directories(ShellEngineRender PRIVATE ${CMAKE_SOURCE_DIR}/include/Render/VulkanImpl)
target_include_directories(ShellEngineRender PRIVATE ${CMAKE_SOURCE_DIR}/include/external)
