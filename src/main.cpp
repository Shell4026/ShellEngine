﻿#include <iostream>
#include <chrono>
#include <thread>

#include "Window/Window.h"
#include "Render/VulkanShader.h"
#include "Render/VulkanShaderBuilder.h"
#include "Render/VulkanRenderer.h"
#include "Render/ShaderLoader.h"
#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Core/Reflaction.hpp"
#include <Core/Util.h>
#include <cassert>
#include <fmt/core.h>
#include "Game/World.h"
#include "Game/GameObject.h"
#include "Game/Component/Transform.h"
#include "Game/Component/MeshRenderer.h"

#include <iostream>
class Base {
	SCLASS(Base)
public:

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
	void DerivedFunction() 
	{ 
		std::cout << "Derived!!\n"; 
	}
};

class NoBase {
public:
	void NoBaseFunction()
	{
		std::cout << "NoBase!!\n";
	}
};

int main(int arg, char* args[]) 
{
	
	Base base;
	Derived derived;
	NoBase nobase, nobase2;
	Base* p = &derived;

	std::cout << derived.GetTypeInfo().GetName() << "\n"; //Derived 출력
	std::cout << Derived::Super::GetStaticTypeInfo().GetName() << "\n"; //Base 출력

	auto real = sh::core::Util::Cast<Derived>(p);
	assert(real != nullptr);
	p->BaseFunction();
	real->DerivedFunction();

	sh::window::Window window;
	window.Create(u8"테스트", 1024, 768);
	auto renderer = sh::render::VulkanRenderer{};
	renderer.Init(window);
	
	sh::render::VulkanShaderBuilder builder{ renderer };
	sh::render::ShaderLoader loader{ &builder };
	auto shader = loader.LoadShader<sh::render::VulkanShader>("triangle.spv", "frag.spv");

	sh::render::Material mat{};
	mat.SetShader(shader.get());

	sh::render::Mesh mesh{};
	mesh.AddMaterial(&mat);

	using namespace sh::game;
	World world{ renderer };
	GameObject* obj = world.AddGameObject("Test");

	auto meshRenderer = obj->AddComponent<MeshRenderer>();
	meshRenderer->SetMesh(mesh);

	world.Start();
	while (window.IsOpen())
	{
		window.ProcessFrame();
		//fmt::print("deltaTime: {}s\n", window.GetDeltaTime());

		sh::window::Event e;
		while (window.PollEvent(e))
		{
			switch (e.type)
			{
			case sh::window::Event::EventType::Close:
				renderer.Clean();
				window.Close();
				break;
			case sh::window::Event::EventType::MousePressed:
				if (e.mouseType == sh::window::Event::MouseType::Left)
				{
					std::cout << "Left\n";
				}
				else if (e.mouseType == sh::window::Event::MouseType::Right)
				{
					std::cout << "Right\n";
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
				if (e.keyType == sh::window::Event::KeyType::Left)
					std::cout << "Left\n";
				break;
			case sh::window::Event::EventType::WindowFocus:
				renderer.Pause(false);
				std::cout << "FocusIn\n";
				break;
			case sh::window::Event::EventType::WindowFocusOut:
				renderer.Pause(true);
				std::cout << "FocusOut\n";
				break;
			}
		}
		world.Update(window.GetDeltaTime());
		renderer.Render(window.GetDeltaTime());
	}
	return 0;
}