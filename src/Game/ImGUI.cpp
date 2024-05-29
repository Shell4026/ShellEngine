﻿#include "ImGUI.h"

#include "Render/VulkanImpl/VulkanFramebuffer.h"

namespace sh::game
{
	bool ImGUI::bInit = false;

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
		bool keyDown = false;
		switch (event.type)
		{
		case window::Event::EventType::WindowFocus:
			io.AddFocusEvent(true); 
			break;
		case window::Event::EventType::WindowFocusOut:
			io.AddFocusEvent(false); 
			break;
		case window::Event::EventType::MousePressed:
			if(event.mouseType == window::Event::MouseType::Left)
				io.AddMouseButtonEvent(0, true);
			if (event.mouseType == window::Event::MouseType::Right)
				io.AddMouseButtonEvent(1, true);
			if (event.mouseType == window::Event::MouseType::Middle)
				io.AddMouseButtonEvent(2, true);
			break;
		case window::Event::EventType::MouseReleased:
			if (event.mouseType == window::Event::MouseType::Left)
				io.AddMouseButtonEvent(0, false);
			if (event.mouseType == window::Event::MouseType::Right)
				io.AddMouseButtonEvent(1, false);
			if (event.mouseType == window::Event::MouseType::Middle)
				io.AddMouseButtonEvent(2, false);
			break;
		case window::Event::EventType::MouseWheelScrolled:
			io.AddMouseWheelEvent(0, window::Event::MouseWheelScrolled::delta);
			break;
		case window::Event::EventType::KeyDown:
			keyDown = true;
			[[fallthrough]];
		case window::Event::EventType::KeyUp:
			switch (event.keyType)
			{
			case window::Event::KeyType::A: [[fallthrough]];
			case window::Event::KeyType::B: [[fallthrough]];
			case window::Event::KeyType::C: [[fallthrough]];
			case window::Event::KeyType::D: [[fallthrough]];
			case window::Event::KeyType::E: [[fallthrough]];
			case window::Event::KeyType::F: [[fallthrough]];
			case window::Event::KeyType::G: [[fallthrough]];
			case window::Event::KeyType::H: [[fallthrough]];
			case window::Event::KeyType::J: [[fallthrough]];
			case window::Event::KeyType::K: [[fallthrough]];
			case window::Event::KeyType::L: [[fallthrough]];
			case window::Event::KeyType::M: [[fallthrough]];
			case window::Event::KeyType::N: [[fallthrough]];
			case window::Event::KeyType::O: [[fallthrough]];
			case window::Event::KeyType::P: [[fallthrough]];
			case window::Event::KeyType::Q: [[fallthrough]];
			case window::Event::KeyType::R: [[fallthrough]];
			case window::Event::KeyType::S: [[fallthrough]];
			case window::Event::KeyType::T: [[fallthrough]];
			case window::Event::KeyType::U: [[fallthrough]];
			case window::Event::KeyType::V: [[fallthrough]];
			case window::Event::KeyType::W: [[fallthrough]];
			case window::Event::KeyType::X: [[fallthrough]];
			case window::Event::KeyType::Y: [[fallthrough]];
			case window::Event::KeyType::Z:
			{
				int keyDif = static_cast<int>(event.keyType) - static_cast<int>(window::Event::KeyType::A);
				io.AddKeyEvent(
					static_cast<ImGuiKey>(ImGuiKey::ImGuiKey_A + keyDif), keyDown);
				if (keyDown) io.AddInputCharacter('a' + keyDif);
				break;
			}
			case window::Event::KeyType::Num0: [[fallthrough]];
			case window::Event::KeyType::Num1: [[fallthrough]];
			case window::Event::KeyType::Num2: [[fallthrough]];
			case window::Event::KeyType::Num3: [[fallthrough]];
			case window::Event::KeyType::Num4: [[fallthrough]];
			case window::Event::KeyType::Num5: [[fallthrough]];
			case window::Event::KeyType::Num6: [[fallthrough]];
			case window::Event::KeyType::Num7: [[fallthrough]];
			case window::Event::KeyType::Num8: [[fallthrough]];
			case window::Event::KeyType::Num9:
			{
				int keyDif = static_cast<int>(event.keyType) - static_cast<int>(window::Event::KeyType::Num0);
				io.AddKeyEvent(static_cast<ImGuiKey>(ImGuiKey::ImGuiKey_0 + keyDif), keyDown);
				if (keyDown) io.AddInputCharacter('0' + keyDif);
				break;
			}
			case window::Event::KeyType::Left:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_LeftArrow, keyDown);
				break;
			case window::Event::KeyType::Up:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_UpArrow, keyDown);
				break;
			case window::Event::KeyType::Right:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_RightArrow, keyDown);
				break;
			case window::Event::KeyType::Down:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_DownArrow, keyDown);
				break;
			case window::Event::KeyType::BackSpace:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Backspace, keyDown);
				break;
			case window::Event::KeyType::Enter:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Enter, keyDown);
				break;
			case window::Event::KeyType::Space:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Space, keyDown);
				if (keyDown) io.AddInputCharacter(' ');
				break;
			case window::Event::KeyType::Esc:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Escape, keyDown);
				break;
			case window::Event::KeyType::Tab:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Tab, keyDown);
				if (keyDown) io.AddInputCharacter('	');
				break;
			case window::Event::KeyType::Delete:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Delete, keyDown);
				break;
			case window::Event::KeyType::Shift:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_ModShift, keyDown);
				break;
			case window::Event::KeyType::Minus:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Minus, keyDown);
				if (keyDown) io.AddInputCharacter('-');
				break;
			case window::Event::KeyType::Equal:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Equal, keyDown);
				if (keyDown) io.AddInputCharacter('=');
				break;
			case window::Event::KeyType::Period:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Period, keyDown);
				if (keyDown) io.AddInputCharacter('`');
				break;
			case window::Event::KeyType::Semicolon:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Semicolon, keyDown);
				if (keyDown) io.AddInputCharacter(';');
				break;
			case window::Event::KeyType::Colon:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Apostrophe, keyDown);
				if (keyDown) io.AddInputCharacter('\'');
				break;
			case window::Event::KeyType::Comma:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Comma, keyDown);
				if (keyDown) io.AddInputCharacter(',');
				break;
			case window::Event::KeyType::LBracket:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_LeftBracket, keyDown);
				if (keyDown) io.AddInputCharacter('[');
				break;
			case window::Event::KeyType::RBracket:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_RightBracket, keyDown);
				if (keyDown) io.AddInputCharacter(']');
				break;
			case window::Event::KeyType::Grave:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_GraveAccent, keyDown);
				if (keyDown) io.AddInputCharacter('.');
				break;
			case window::Event::KeyType::Slash:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Slash, keyDown);
				if (keyDown) io.AddInputCharacter('/');
				break;
			}
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
	bool ImGUI::IsInit()
	{
		return bInit;
	}
}