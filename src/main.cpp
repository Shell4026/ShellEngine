﻿#include <iostream>
#include <chrono>
#include <thread>

#include "Window/Window.h"
#include "Render/VulkanShader.h"
#include "Render/VulkanRenderer.h"
#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Render/RenderTexture.h"
#include "Core/Reflection.hpp"
#include <Core/Util.h>
#include <Core/GarbageCollection.h>
#include "Core/ModuleLoader.h"
#include <cassert>
#include <fmt/core.h>
#include "Game/Input.h"
#include "Game/ResourceManager.hpp"
#include "Game/VulkanShaderBuilder.h"
#include "Game/ShaderLoader.h"
#include "Game/TextureLoader.h"
#include "Game/ModelLoader.h"
#include "Game/World.h"
#include "Game/GameObject.h"
#include "Game/Component/Transform.h"
#include "Game/Component/MeshRenderer.h"
#include "Game/Component/UniformTest.h"
#include "Game/Component/Camera.h"
#include "Game/ComponentModule.h"
#include "Game/ImGUI.h"

#include "Editor/EditorUI.h"
#include "Editor/Project.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

class NoBase {
public:
	int a = 123;
	void NoBaseFunction()
	{
		std::cout << "NoBase!!\n";
	}
};

class Base : public sh::core::SObject {
	SCLASS(Base)
public:
	void BaseFunction() 
	{ 
		std::cout << "Base!!\n"; 
	}
};

class Derived : public Base
{
	SCLASS(Derived)
public:
	PROPERTY(ptr)
	Derived* ptr;

	std::string name;
public:
	Derived(std::string_view name) :
		name(name)
	{
	}
	~Derived()
	{

	}
	void DerivedFunction()
	{
		std::cout << "Derived!!\n"; 
	}
};

int main(int arg, char* args[]) 
{
	sh::core::ModuleLoader moduleLoader;
	void* modulePtr = moduleLoader.Load("ShellEngineUser");
	sh::game::ComponentModule* componentModule = reinterpret_cast<sh::game::ComponentModule*>(modulePtr);
	for (auto& components : componentModule->GetComponents())
	{
		fmt::print("Load Component: {}\n", components.first);
	}
	//GC초기화
	auto gc = sh::core::GarbageCollection::GetInstance();
	
	sh::window::Window window;
	window.Create(u8"테스트", 1024, 768, sh::window::Window::Style::Resize);
	window.SetFps(144);

	auto renderer = sh::render::VulkanRenderer{};
	renderer.Init(window);
	renderer.SetViewport({ 150.f, 0.f }, { window.width - 150.f, window.height - 180});

	using namespace sh::game;

	World world{ renderer, *componentModule };
	
	VulkanShaderBuilder builder{ renderer };

	ShaderLoader loader{ &builder };
	TextureLoader texLoader{ renderer };
	ModelLoader modelLoader{ renderer };

	auto shader = world.shaders.AddResource("Triangle", loader.LoadShader<sh::render::VulkanShader>("shaders/vert.spv", "shaders/frag.spv"));
	auto mat = world.materials.AddResource("Material", sh::render::Material{ shader });
	auto mat2 = world.materials.AddResource("Material2", sh::render::Material{ shader });
	auto mesh = world.meshes.AddResource("Mesh", sh::render::Mesh{});
	auto mesh2 = world.meshes.AddResource("Mesh2", modelLoader.Load("model/test.obj"));
	auto tex = world.textures.AddResource("Texture0", texLoader.Load("textures/버터고양이.jpg"));
	auto tex2 = world.textures.AddResource("Texture1", texLoader.Load("textures/cat.jpg"));
	auto tex3 = world.textures.AddResource("Texture2", texLoader.Load("textures/viking_room.png"));
	auto renderTex = world.textures.AddResource("RenderTexture", std::make_unique<sh::render::RenderTexture>());
	renderTex->Build(renderer);

	shader->AddAttribute<glm::vec2>("uvs", 1);

	shader->AddUniform<glm::mat4>("model", 0, sh::render::Shader::ShaderStage::Vertex);
	shader->AddUniform<glm::mat4>("view", 0, sh::render::Shader::ShaderStage::Vertex);
	shader->AddUniform<glm::mat4>("proj", 0, sh::render::Shader::ShaderStage::Vertex);

	shader->AddUniform<glm::vec3>("offset1", 1, sh::render::Shader::ShaderStage::Vertex);
	shader->AddUniform<float>("offset2", 1, sh::render::Shader::ShaderStage::Vertex);
	shader->AddUniform<sh::render::Texture>("tex", 2, sh::render::Shader::ShaderStage::Fragment);
	shader->Build();

	mat->SetVector("offset1", glm::vec4(0.f, 0.0f, 0.f, 0.f));
	mat->SetFloat("offset2", 0.f);
	mat->SetTexture("tex", tex);

	mat2->SetTexture("tex", tex3);

	mesh->SetVertex({ 
		{-0.5f, 0.0f, -0.5f}, 
		{-0.5f, 0.0f, 0.5f},
		{0.5f, 0.0f, 0.5f},
		{0.5f, 0.0f, -0.5f}
	});
	mesh->SetIndices({
		0, 1, 2, 2, 3, 0
	});
	mesh->SetAttribute(sh::render::ShaderAttribute<glm::vec2>{"uvs", {
		{0.0f, 0.0f},
		{0.0f, 1.0f},
		{1.0f, 1.0f},
		{1.0f, 0.0f}
	}});
	mesh->Build(renderer);

	GameObject* obj = world.AddGameObject("Test");
	GameObject* obj2 = world.AddGameObject("Test2");

	//obj2->AddComponent(componentModule->GetComponent("ComponentTest")->New());

	auto transform = obj->transform;
	transform->SetRotation({ -90.f, 0.f, 0.f });

	auto meshRenderer = obj->AddComponent<MeshRenderer>();
	meshRenderer->SetMesh(*mesh2);
	meshRenderer->SetMaterial(*mat2);
	
	obj->AddComponent<UniformTest>();

	auto meshRenderer2 = obj2->AddComponent<MeshRenderer>();
	meshRenderer2->SetMesh(*mesh);
	meshRenderer2->SetMaterial(*mat);

	GameObject* cam = world.AddGameObject("Camera");
	cam->transform->SetPosition(glm::vec3(2.f, 2.f, 2.f));
	Camera* cameraComponent = cam->AddComponent<Camera>();
	//cameraComponent->renderTexture = sh::core::reflection::Cast<sh::render::RenderTexture>(renderTex);
	
	GameObject* cam2 = world.AddGameObject("Camera2");
	cam2->transform->SetPosition(glm::vec3(-2.f, 2.f, -2.f));
	//cam2->AddComponent<Camera>()->renderTexture = sh::core::reflection::Cast<sh::render::RenderTexture>(renderTex);
	//cam2->GetComponent<Camera>()->SetDepth(-2);

	world.Start();
	world.SetMainCamera(cameraComponent);

	//ImGUI gui(window, renderer);
	//gui.Init();

	//sh::editor::EditorUI editorUi(world, gui);

	std::atomic<bool> gameThreadReady{ false };
	std::atomic<bool> renderThreadReady{ true };
	std::condition_variable cv;
	std::mutex mu;

	std::thread renderThread
	{
		[&]
		{
			while (window.IsOpen())
			{
				//gui.Update();
				//editorUi.Render();
				//gui.Render();
				renderer.Render(window.GetDeltaTime());
				if (gameThreadReady.load(std::memory_order::memory_order_acquire))
				{
					renderer.SyncGameThread();
					gameThreadReady.store(false, std::memory_order::memory_order_release);
					renderThreadReady.store(true, std::memory_order::memory_order_release);
					cv.notify_one();
				}
				//gui.Render();
			}
		}
	};

	while (window.IsOpen())
	{
		window.ProcessFrame();
		
		if (renderThreadReady.load(std::memory_order_acquire) == false)
		{
			auto start = std::chrono::high_resolution_clock::now();
			std::unique_lock<std::mutex> lock{ mu };
			cv.wait(lock, [&renderThreadReady] {return renderThreadReady.load(std::memory_order_acquire); });
			std::cout << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() << '\n';
		}
		renderThreadReady.store(false, std::memory_order::memory_order_release);

		std::string deltaTime = std::to_string(window.GetDeltaTime());
		deltaTime.erase(deltaTime.begin() + 5, deltaTime.end());
		window.SetTitle("ShellEngine [DeltaTime:" + deltaTime + "ms]");
		sh::window::Event e;
		while (window.PollEvent(e))
		{
			Input::Update(e);
			//gui.ProcessEvent(e);

			switch (e.type)
			{
			case sh::window::Event::EventType::Close:
				//gui.Clean();
				world.Clean();
				window.Close();
				if (renderThread.joinable())
					renderThread.join();
				renderer.Clean();
				break;
			case sh::window::Event::EventType::Resize:
				if (window.width == 0)
				{
					renderer.Pause(true);
				}
				else
				{
					renderer.Pause(false);
					renderer.SetViewport({ 150.f, 0.f }, { window.width - 150.f, window.height - 180 });
					//gui.Resize();
				}
				break;
			case sh::window::Event::EventType::MousePressed:
				if (e.mouseType == sh::window::Event::MouseType::Left)
				{
					mat->SetTexture("tex", tex);
				}
				else if (e.mouseType == sh::window::Event::MouseType::Right)
				{
					mat->SetTexture("tex", tex2);
				}
				else
				{
					std::cout << "Middle\n";
				}
				break;
			case sh::window::Event::EventType::MouseWheelScrolled:
				std::cout << sh::window::Event::MouseWheelScrolled::delta << '\n';
				break;
			case sh::window::Event::EventType::KeyDown:
				if (e.keyType == sh::window::Event::KeyType::Enter)
				{
					world.meshes.DestroyResource("Mesh");
				}
				break;
			case sh::window::Event::EventType::WindowFocus:
				window.SetFps(144);
				std::cout << "FocusIn\n";
				break;
			case sh::window::Event::EventType::WindowFocusOut:
				window.SetFps(30);
				std::cout << "FocusOut\n";
				break;
			}
		}
		//editorUi.Update();
		world.Update(window.GetDeltaTime());

		gc->Update();

		gameThreadReady.store(true, std::memory_order::memory_order_release);
	}
	return 0;
}