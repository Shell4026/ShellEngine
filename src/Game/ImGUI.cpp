#include "ImGUI.h"

#include "Render/VulkanImpl/VulkanFramebuffer.h"

namespace sh::game
{
	static void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

	ImGUI::ImGUI(window::Window& window, render::VulkanRenderer& renderer) :
		window(window), renderer(renderer)
	{
	}

	ImGUI::~ImGUI()
	{
		Clean();
	}
	void ImGUI::Clean()
	{
		if (!bInit)
			return;
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		bInit = false;
	}

	void ImGUI::Init()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

		ImGui_ImplWin32_Init(window.GetNativeHandle());

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = renderer.GetInstance();
		initInfo.PhysicalDevice = renderer.GetGPU();
		initInfo.Device = renderer.GetDevice();
		initInfo.QueueFamily = renderer.GetGraphicsQueueIdx();
		initInfo.Queue = renderer.GetGraphicsQueue();
		initInfo.DescriptorPool = renderer.GetDescriptorPool();
		initInfo.RenderPass = static_cast<const render::impl::VulkanFramebuffer*>(renderer.GetMainFramebuffer())->GetRenderPass();
		initInfo.MinImageCount = 2;
		initInfo.ImageCount = 2;
		initInfo.MSAASamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		initInfo.Subpass = 0;
		initInfo.PipelineCache = nullptr;
		initInfo.CheckVkResultFn = check_vk_result;
		initInfo.UseDynamicRendering = false;
		ImGui_ImplVulkan_Init(&initInfo);
		ImGui_ImplVulkan_CreateFontsTexture();

		renderer.AddDrawCall(
		[&]()
		{
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderer.GetCommandBuffer()); 
		});

		bInit = true;
	}

	void ImGUI::ProcessEvent(window::Event event)
	{
		ImGuiIO& io = ImGui::GetIO();
		switch (event.type)
		{
		case window::Event::EventType::MousePressed:
			if(event.mouseType == window::Event::MouseType::Left)
				io.AddMouseButtonEvent(0, true);
			break;
		case window::Event::EventType::MouseReleased:
			if (event.mouseType == window::Event::MouseType::Left)
				io.AddMouseButtonEvent(0, false);
			break;
		}
	}

	void ImGUI::Update()
	{
		if (!bInit)
			return;
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();
	}
	void ImGUI::Render()
	{
		if (!bInit)
			return;
		ImGui::Render();
	}
}