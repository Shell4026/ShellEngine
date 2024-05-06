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
	PROPERTY(ptrs)
	std::vector<Derived*> ptrs;
	PROPERTY(ptrs2)
	std::vector<Derived*> ptrs2;
	PROPERTY(a)
	int a = 123;
	PROPERTY(ptrNoBase)
	NoBase* ptrNoBase = nullptr;
	//PROPERTY(mapPtr)
	//std::map<Derived*, int> mapPtr;

	std::string name;
public:
	void DerivedFunction()
	{
		sh::core::reflection::IsContainer<std::vector<Derived*>>::value;
		int Derived::* ptr = &Derived::a;
		auto a = this->*ptr;
		std::cout << "Derived!!\n"; 
	}
};



int main(int arg, char* args[]) 
{
	Base base;
	Derived derived, derived2;
	NoBase nobase;
	Base* p = &derived;

	sh::core::GC gc;
	derived.SetGC(gc);
	derived.name = "abc";

	//sh::core::reflection::IsMap<std::map<int, bool>>::value;

	auto& type = Derived::GetStaticType();
	auto& props = Derived::GetStaticType().GetProperties();

	derived.a = 1;
	derived2.a = 2;

	auto prop = type.GetProperty("a");
	int a1 = prop->Get<int>(&derived);
	int a2 = prop->Get<int>(&derived2);

	auto property = Derived::GetStaticType().GetProperty("a");
	{
		Derived* derivedTemp = new Derived;
		derivedTemp->SetGC(gc);
		derivedTemp->name = "garbage";

		Derived* derivedTemp2 = new Derived;
		derivedTemp2->SetGC(gc);
		derivedTemp2->name = "garbage2";

		derived.ptr = derivedTemp;
		derived.ptrs.push_back(derivedTemp);
		derived.ptrs.push_back(derivedTemp2);

		auto vproperty = Derived::GetStaticType().GetProperty("ptrs");
		for (auto it = vproperty->Begin(&derived); it != vproperty->End(&derived); ++it)
		{
			std::cout << (*it.Get<Derived*>())->name << '\n';
		}
		delete derivedTemp; //derived.ptr = nullptr, derived.ptrs[0] = nullptr이 된다.
		delete derivedTemp2;
	}
	
	std::string_view str = sh::core::reflection::GetTypeName<const int*>();

	std::cout << derived.GetType().GetName() << "\n"; //Derived 출력
	std::cout << Derived::Super::GetStaticType().GetName() << "\n"; //Base 출력

	std::vector<int> v;

	auto real = sh::core::reflection::Cast<Derived>(p);
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