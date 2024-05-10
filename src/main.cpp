#include <iostream>
#include <chrono>
#include <thread>

#include "Window/Window.h"
#include "Render/VulkanShader.h"
#include "Render/VulkanShaderBuilder.h"
#include "Render/VulkanRenderer.h"
#include "Render/ShaderLoader.h"
#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Core/Reflection.hpp"
#include <Core/Util.h>
#include <Core/GC.h>
#include <cassert>
#include <fmt/core.h>
#include "Game/World.h"
#include "Game/GameObject.h"
#include "Game/Component/Transform.h"
#include "Game/Component/MeshRenderer.h"

#include <iostream>

template<typename T>
struct VectorDepth
{
	static const int value = 0;
};

template<typename T>
struct VectorDepth<std::vector<T>>
{
	static const int value = VectorDepth<T>::value + 1;
};


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
	void DerivedFunction()
	{
		std::cout << "Derived!!\n"; 
	}
};



int main(int arg, char* args[]) 
{
	sh::core::GC gc;

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
	World world{ renderer, gc };
	GameObject* obj = world.AddGameObject("Test");

	auto meshRenderer = obj->AddComponent<MeshRenderer>();
	meshRenderer->SetMesh(mesh);

	world.Start();

	Derived derived{ "good" };
	derived.SetGC(gc);

	Derived* temp = new Derived{ "trash" };
	temp->SetGC(gc);

	derived.ptr = temp;
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
				if (e.keyType == sh::window::Event::KeyType::Enter)
					delete temp;
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
		if(sh::core::IsValid(derived.ptr))
			std::cout << derived.ptr->name << '\n';
		else
			std::cout << "Delete!!!" << '\n';

		world.Update(window.GetDeltaTime());
		renderer.Render(window.GetDeltaTime());
		gc.Update();
	}
	return 0;
}