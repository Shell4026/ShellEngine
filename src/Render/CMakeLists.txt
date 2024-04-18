message("Build ShellEngine Renderer")
file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB VULKAN_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/VulkanImpl/*.cpp)

add_library(ShellEngineRender SHARED ${SRC} ${VULKAN_SRC})
add_library(ShellEngine::Render ALIAS ShellEngineRender)

target_link_libraries(ShellEngineRender PUBLIC ShellEngine::Core)
target_link_libraries(ShellEngineRender PUBLIC ShellEngine::Window)

find_package(Vulkan)
if(NOT Vulkan_FOUND)
	#불칸 경로 지정
	message("Couldn't found Vulkan! Attempting to locate Vulkan SDK using manual path...")
	if(WIN32)
		set(Vulkan_INCLUDE_DIR "C:/VulkanSDK/1.3.280.0/Include")
		set(Vulkan_LIBRARY "C:/VulkanSDK/1.3.280.0/Lib/vulkan-1.lib")
	endif()
	find_package(Vulkan REQUIRED)
endif()
target_link_libraries(ShellEngineRender PUBLIC Vulkan::Vulkan)

include(FetchContent)
FetchContent_Declare(
  GLM
  GIT_REPOSITORY "https://github.com/g-truc/glm"
  GIT_TAG "1.0.1"
  )
FetchContent_MakeAvailable(GLM)
target_link_libraries(ShellEngineRender PUBLIC glm)

target_include_directories(ShellEngineRender PUBLIC ${Vulkan_INCLUDE_DIR})
target_include_directories(ShellEngineRender PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(ShellEngineRender PRIVATE ${CMAKE_SOURCE_DIR}/include/Render)
target_include_directories(ShellEngineRender PRIVATE ${CMAKE_SOURCE_DIR}/include/Render/VulkanImpl)