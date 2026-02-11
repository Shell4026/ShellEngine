#include "ImGUImpl.h"

#include "Render/VulkanImpl/VulkanRenderer.h"
#include "Render/VulkanImpl/VulkanContext.h"
#include "Render/VulkanImpl/VulkanQueueManager.h"
#include "Render/VulkanImpl/VulkanCommandBufferPool.h"
#include "Render/VulkanImpl/VulkanCommandBuffer.h"
#include "Render/VulkanImpl/VulkanRenderer.h"
#include "Render/VulkanImpl/VulkanSwapChain.h"

#include "Core/ThreadSyncManager.h"

#include "External/imgui/backends/imgui_impl_vulkan.h"
#include "External/imgui/ImGuizmo.h"
#include <iostream>

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

	ImGUImpl::ImGUImpl(window::Window& window, render::Renderer& renderer) :
		window(window), renderer(renderer),
		drawData(),
		bInit(false), bDirty(false)
	{
	}

	ImGUImpl::~ImGUImpl()
	{
		Clean();
	}
	SH_GAME_API void ImGUImpl::Clean()
	{
		if (!bInit)
			return;
		ImGui_ImplVulkan_Shutdown();
		ImGui::DestroyContext();
		bInit = false;
	}

	SH_GAME_API void ImGUImpl::ClearDrawData()
	{
		// DrawData.Clear()는 drawData*를 해제 하지 않기에 수동 소멸시켜야 함.
		for (int i = 0; i < drawData.CmdListsCount; ++i)
			IM_DELETE(drawData.CmdLists[i]);
		drawData.Clear();
		drawData.CmdLists.clear();
	}

	void ImGUImpl::WindowInit()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |=
			ImGuiBackendFlags_::ImGuiBackendFlags_HasMouseCursors;
		io.BackendPlatformName = "ShellEngine";
		io.DisplaySize = ImVec2{ static_cast<float>(window.width), static_cast<float>(window.height) };
		io.ConfigFlags |= 
			ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable | 
			ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableKeyboard;
	}

	SH_GAME_API void ImGUImpl::Init()
	{
		if (bInit)
			return;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		//ImGui_ImplWin32_Init(window.GetNativeHandle());
		WindowInit();

		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0x1100, 0x11FF, // 한글 자모
			0x3130, 0x318F, // 한글 호환 자모
			0xAC00, 0xD7AF, // 한글 음절
			0x0000
		};
		ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/Pretendard-Medium.otf", 16.0f, nullptr, ranges);

		if (renderer.GetContext()->GetRenderAPIType() == render::RenderAPI::Vulkan)
		{
			render::vk::VulkanContext& vkContext = static_cast<render::vk::VulkanContext&>(*renderer.GetContext());
			ImGui_ImplVulkan_InitInfo initInfo{};
			initInfo.Instance = vkContext.GetInstance();
			initInfo.PhysicalDevice = vkContext.GetGPU();
			initInfo.Device = vkContext.GetDevice();
			initInfo.QueueFamily = vkContext.GetQueueManager().GetFamilyIndex(render::vk::VulkanQueueManager::Role::Graphics);
			initInfo.Queue = vkContext.GetQueueManager().GetQueue(render::vk::VulkanQueueManager::Role::Graphics);
			initInfo.DescriptorPool = &vkContext.GetDescriptorPool();
			//initInfo.RenderPass = static_cast<const render::vk::VulkanFramebuffer*>(vkContext.GetMainFramebuffer())->GetRenderPass()->GetVkRenderPass();
			initInfo.MinImageCount = 2;
			initInfo.ImageCount = 2;
			initInfo.MSAASamples = vkContext.GetSampleCount();
			initInfo.Subpass = 0;
			initInfo.PipelineCache = nullptr;
			initInfo.CheckVkResultFn = check_vk_result;
			initInfo.UseDynamicRendering = true;

			VkFormat colorFormat = vkContext.GetSwapChain().GetSwapChainImages()[0].GetFormat();;

			VkPipelineRenderingCreateInfoKHR ci{};
			ci.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
			ci.colorAttachmentCount = 1;
			ci.pColorAttachmentFormats = &colorFormat;
			ci.depthAttachmentFormat = vkContext.GetSwapChain().GetSwapChainDepthImages()[0].GetFormat();
			ci.stencilAttachmentFormat = vkContext.GetSwapChain().GetSwapChainDepthImages()[0].GetFormat();

			initInfo.PipelineRenderingCreateInfo = ci;
			ImGui_ImplVulkan_Init(&initInfo);
			ImGui_ImplVulkan_CreateFontsTexture();
		}
		bInit = true;
	}

	SH_GAME_API void ImGUImpl::Resize()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2{ static_cast<float>(window.width), static_cast<float>(window.height) };
	}

	SH_GAME_API void ImGUImpl::ProcessEvent(window::Event event)
	{
		ImGuiIO& io = ImGui::GetIO();

		io.MousePos = ImVec2{ static_cast<float>(window::Event::MousePosition::mouseX), static_cast<float>(window::Event::MousePosition::mouseY) };
		io.DeltaTime = window.GetDeltaTime();

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
		{
			const auto mouseType = std::get<1>(event.data);
			if (mouseType == window::Event::MouseType::Left)
				io.AddMouseButtonEvent(0, true);
			if (mouseType == window::Event::MouseType::Right)
				io.AddMouseButtonEvent(1, true);
			if (mouseType == window::Event::MouseType::Middle)
				io.AddMouseButtonEvent(2, true);
			break;
		}
		case window::Event::EventType::MouseReleased:
		{
			const auto mouseType = std::get<1>(event.data);
			if (mouseType == window::Event::MouseType::Left)
				io.AddMouseButtonEvent(0, false);
			if (mouseType == window::Event::MouseType::Right)
				io.AddMouseButtonEvent(1, false);
			if (mouseType == window::Event::MouseType::Middle)
				io.AddMouseButtonEvent(2, false);
			break;
		}
		case window::Event::EventType::MouseWheelScrolled:
			io.AddMouseWheelEvent(0, window::Event::MouseWheelScrolled::delta);
			break;
		case window::Event::EventType::InputText:
		{
			const uint32_t unicode = std::get<3>(event.data).unicode;
			io.AddInputCharacter(unicode);
			break;
		}
		case window::Event::EventType::KeyDown:
			keyDown = true;
			[[fallthrough]];
		case window::Event::EventType::KeyUp:
		{
			const auto keyType = std::get<2>(event.data).keyType;
			const bool bCaps = std::get<2>(event.data).capsLock;
			switch (keyType)
			{
			case window::Event::KeyType::A: [[fallthrough]];
			case window::Event::KeyType::B: [[fallthrough]];
			case window::Event::KeyType::C: [[fallthrough]];
			case window::Event::KeyType::D: [[fallthrough]];
			case window::Event::KeyType::E: [[fallthrough]];
			case window::Event::KeyType::F: [[fallthrough]];
			case window::Event::KeyType::G: [[fallthrough]];
			case window::Event::KeyType::H: [[fallthrough]];
			case window::Event::KeyType::I: [[fallthrough]];
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
				const int keyDif = static_cast<int>(keyType) - static_cast<int>(window::Event::KeyType::A);
				io.AddKeyEvent(
					static_cast<ImGuiKey>(ImGuiKey::ImGuiKey_A + keyDif), keyDown);
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
			case window::Event::KeyType::Home:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Home, keyDown);
				break;
			case window::Event::KeyType::Insert:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Insert, keyDown);
				break;
			case window::Event::KeyType::End:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_End, keyDown);
				break;
			case window::Event::KeyType::PageUp:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_PageUp, keyDown);
				break;
			case window::Event::KeyType::PageDown:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_PageDown, keyDown);
				break;
			case window::Event::KeyType::Print:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_PrintScreen, keyDown);
				break;
			case window::Event::KeyType::Scroll:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_ScrollLock, keyDown);
				break;
			case window::Event::KeyType::Pause:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_Pause, keyDown);
				break;
			case window::Event::KeyType::Shift:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_ModShift, keyDown);
				io.AddKeyEvent(ImGuiKey::ImGuiMod_Shift, keyDown);
				break;
			case window::Event::KeyType::LCtrl:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_LeftCtrl, keyDown);
				io.AddKeyEvent(ImGuiKey::ImGuiMod_Ctrl, keyDown);
				break;
			case window::Event::KeyType::RCtrl:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_RightCtrl, keyDown);
				break;
			case window::Event::KeyType::LAlt:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_LeftAlt, keyDown);
				io.AddKeyEvent(ImGuiKey::ImGuiMod_Alt, keyDown);
				break;
			case window::Event::KeyType::RAlt:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_RightAlt, keyDown);
				break;
			case window::Event::KeyType::CapsLock:
				io.AddKeyEvent(ImGuiKey::ImGuiKey_CapsLock, keyDown);
				break;
			case window::Event::KeyType::F1: [[fallthrough]];
			case window::Event::KeyType::F2: [[fallthrough]];
			case window::Event::KeyType::F3: [[fallthrough]];
			case window::Event::KeyType::F4: [[fallthrough]];
			case window::Event::KeyType::F5: [[fallthrough]];
			case window::Event::KeyType::F6: [[fallthrough]];
			case window::Event::KeyType::F7: [[fallthrough]];
			case window::Event::KeyType::F8: [[fallthrough]];
			case window::Event::KeyType::F9: [[fallthrough]];
			case window::Event::KeyType::F10: [[fallthrough]];
			case window::Event::KeyType::F11: [[fallthrough]];
			case window::Event::KeyType::F12:
			{
				int keyDif = static_cast<int>(keyType) - static_cast<int>(window::Event::KeyType::F1);
				io.AddKeyEvent(static_cast<ImGuiKey>(ImGuiKey::ImGuiKey_F1 + keyDif), keyDown);
				break;
			}
			}
			break;
		}
		}
	}

	SH_GAME_API void ImGUImpl::Begin()
	{
		if (!bInit)
			return;
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = window.GetDeltaTime();

		ImGui_ImplVulkan_NewFrame();
		//ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
		//ImGui::ShowDemoWindow();
		SyncDirty();
	}
	SH_GAME_API void ImGUImpl::End()
	{
		if (!bInit)
			return;

		ImGui::Render();
	}
	SH_GAME_API bool ImGUImpl::IsInit() const
	{
		return bInit;
	}
	SH_GAME_API auto ImGUImpl::GetContext() const -> ImGuiContext*
	{
		return ImGui::GetCurrentContext();
	}

	SH_GAME_API auto ImGUImpl::GetDrawData() -> ImDrawData&
	{
		return drawData;
	}

	SH_GAME_API void ImGUImpl::SyncDirty()
	{
		if (bDirty)
			return;

		core::ThreadSyncManager::PushSyncable(*this, render::Renderer::SYNC_PRIORITY + 1);

		bDirty = true;
	}
	SH_GAME_API void ImGUImpl::Sync()
	{
		ImDrawData* src = ImGui::GetDrawData();
		if (src == nullptr || !src->Valid)
			return;

		ClearDrawData();

		// CmdLists 제외 복사
		ImVector<ImDrawList*> temp;
		temp.swap(src->CmdLists);
		drawData = *src;
		temp.swap(src->CmdLists);

		// CmdLists 요소 복사
		drawData.CmdLists.resize(src->CmdLists.Size);
		for (int i = 0; i < drawData.CmdListsCount; ++i)
		{
			ImDrawList* copy = src->CmdLists[i]->CloneOutput();
			drawData.CmdLists[i] = copy;
		}

		bDirty = false;
	}
}//namespace