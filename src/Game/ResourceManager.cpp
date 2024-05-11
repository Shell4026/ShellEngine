#include "ResourceManager.h"

#include "Render/VulkanRenderer.h"
#include "Render/VulkanShaderBuilder.h"
#include "Render/ShaderBuilder.h"
#include "Render/ShaderLoader.h"

namespace sh::game
{
	ResourceManager::ResourceManager(sh::core::GC& gc) :
		gc(gc)
	{

	}

	void ResourceManager::Clean()
	{
		shaders.clear();
		mats.clear();
	}

	auto ResourceManager::AddShader(std::string_view _name, std::unique_ptr<sh::render::Shader>&& shader) -> sh::render::Shader*
	{
		std::string name{ _name };
		
		int idx = 0;
		auto it = shaders.find(name);
		while (it != shaders.end())
		{
			name += std::to_string(idx);
			it = shaders.find(name);
		}

		shader->SetGC(gc);
		return shaders.insert({ name, std::move(shader) }).first->second.get();
	}

	bool ResourceManager::DestroyShader(std::string_view _name)
	{
		std::string name{ _name };
		auto it = shaders.find(name);
		if (it == shaders.end())
			return false;

		it->second.reset();

		return true;
	}

	auto ResourceManager::GetShader(std::string_view name) -> sh::render::Shader*
	{
		auto it = shaders.find(std::string{ name });
		if (it == shaders.end())
			return nullptr;

		return it->second.get();
	}

	auto ResourceManager::AddMaterial(std::string_view _name, sh::render::Material&& mat) -> sh::render::Material*
	{
		std::string name{ _name };

		int idx = 0;
		auto it = mats.find(name);
		while (it != mats.end())
		{
			name += std::to_string(idx);
			it = mats.find(name);
		}

		auto ptr = mats.insert({ name, std::make_unique<sh::render::Material>(std::move(mat)) }).first->second.get();
		ptr->SetGC(gc);
		return ptr;
	}

	auto ResourceManager::AddMaterial(std::string_view _name, std::unique_ptr<sh::render::Material>&& mat) -> sh::render::Material*
	{
		std::string name{ _name };
		int idx = 0;
		auto it = mats.find(name);
		while (it != mats.end())
		{
			name += std::to_string(idx);
			it = mats.find(name);
		}

		mat->SetGC(gc);
		return mats.insert({ name, std::move(mat) }).first->second.get();
	}

	bool ResourceManager::DestroyMaterial(std::string_view _name)
	{
		std::string name{ _name };
		auto it = mats.find(name);
		if (it == mats.end())
			return false;

		it->second.reset();

		return true;
	}

	auto ResourceManager::GetMaterial(std::string_view name) -> sh::render::Material*
	{
		auto it = mats.find(std::string{ name });
		if (it == mats.end())
			return nullptr;

		return it->second.get();
	}
}