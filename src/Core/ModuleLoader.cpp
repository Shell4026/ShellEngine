#include "PCH.h"
#include "ModuleLoader.h"

#include <iostream>
#include <functional>
#if _WIN32
#include <Windows.h>
#elif __linux__
#include <dlfcn.h>
#endif
namespace sh::core
{
	auto ModuleLoader::Load(std::string_view moduleName) -> void*
	{
		std::string name{ moduleName };
#if _WIN32
		name += ".dll";
		HMODULE handle = LoadLibraryA(TEXT(name.c_str()));
		if (handle == nullptr)
		{
			std::cout << "Error load module: " << name << '\n';
			return nullptr;
		}

		std::function<void()> fnInit{ GetProcAddress(handle, "Init") };
		fnInit();

		void* (*fnGetModule)() = (void* (*)())GetProcAddress(handle, "GetModule");
		void* ptr = fnGetModule();
#elif __linux__
		name = "./lib" + name + ".so";
		void* handle = dlopen(name.c_str(), RTLD_LAZY);
		if (!handle) {
			std::cout << "Error load module: " << name << '\n';
			return nullptr;
		}

		std::function<void()> fnInit{ reinterpret_cast<void*(*)()>(dlsym(handle, "Init")) };
		fnInit();

		void* (*fnGetModule)() = (void* (*)())dlsym(handle, "GetModule");
		void* ptr = fnGetModule();
#endif
		return ptr;
	}
}