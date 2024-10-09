#pragma once

#include "Export.h"

#include "Core/Reflection.hpp"
#include "Core/NonCopyable.h"
#include <Core/ICloneable.h>

#include <initializer_list>
#include <vector>
#include <string>
#include <string_view>
#include <type_traits>
#include <memory>

namespace sh::render
{
	class ShaderAttributeBase : public core::INonCopyable, public core::ICloneable<ShaderAttributeBase>
	{
	private:
		std::string attributeName;
	protected:
		std::string mTypeName;
	public:
		const std::string& name;
		const std::string& typeName;
		const bool isInteger;
	public:
		SH_RENDER_API ShaderAttributeBase(std::string_view name, bool isInteger = false);
		SH_RENDER_API virtual ~ShaderAttributeBase() = default;
		SH_RENDER_API ShaderAttributeBase(ShaderAttributeBase&& other) noexcept;

		SH_RENDER_API virtual auto GetData() const -> const void* = 0;
		SH_RENDER_API virtual auto GetSize() const -> size_t = 0;
		SH_RENDER_API virtual auto GetStride() const -> size_t = 0;
	};

	template<typename T>
	class ShaderAttribute : public ShaderAttributeBase
	{
	private:
		std::vector<T> data;
		size_t stride;
	public:
		ShaderAttribute(std::string_view name);
		ShaderAttribute(std::string_view name, std::initializer_list<T>&& datas);
		ShaderAttribute(std::string_view name, const std::vector<T>& data);
		ShaderAttribute(std::string_view name, std::vector<T>&& data);
		~ShaderAttribute() = default;

		ShaderAttribute(ShaderAttribute&& other) noexcept;

		void SetData(std::initializer_list<T>&& datas);
		void SetData(const std::vector<T>& data);
		void SetData(std::vector<T>&& data);

		auto GetData() const -> const void* override;
		auto GetSize() const -> size_t override;
		auto GetStride() const -> size_t override;
		
		auto Clone() const -> std::unique_ptr<ShaderAttributeBase> override;
	};

	template<typename T>
	inline ShaderAttribute<T>::ShaderAttribute(std::string_view name) :
		ShaderAttributeBase(name, std::is_integral_v<T>), 
		stride(sizeof(T))
	{
		mTypeName = core::reflection::GetTypeName<T>();
	}
	template<typename T>
	inline ShaderAttribute<T>::ShaderAttribute(std::string_view name, std::initializer_list<T>&& datas) :
		ShaderAttributeBase(name, std::is_integral_v<T>),
		stride(sizeof(T))
	{
		SetData(datas);
		mTypeName = core::reflection::GetTypeName<T>();
	}
	template<typename T>
	inline ShaderAttribute<T>::ShaderAttribute(std::string_view name, const std::vector<T>& data) :
		ShaderAttributeBase(name, std::is_integral_v<T>),
		data(data), stride(sizeof(T))
	{
		mTypeName = core::reflection::GetTypeName<T>();
	}
	template<typename T>
	inline ShaderAttribute<T>::ShaderAttribute(std::string_view name, std::vector<T>&& data) :
		ShaderAttributeBase(name, std::is_integral_v<T>),
		data(std::move(data)), stride(sizeof(T))
	{
		mTypeName = core::reflection::GetTypeName<T>();
	}
	template<typename T>
	inline ShaderAttribute<T>::ShaderAttribute(ShaderAttribute&& other) noexcept :
		ShaderAttributeBase(std::move(other)),
		data(std::move(other.data)), stride(sizeof(T))
	{
		mTypeName = std::move(other.typeName);
	}
	template<typename T>
	inline void ShaderAttribute<T>::SetData(std::initializer_list<T>&& datas)
	{
		for (auto& data : datas)
		{
			this->data.push_back(std::move(data));
		}
	}
	template<typename T>
	inline void ShaderAttribute<T>::SetData(const std::vector<T>& data)
	{
		this->data = data;
	}
	template<typename T>
	inline void ShaderAttribute<T>::SetData(std::vector<T>&& data)
	{
		this->data = std::move(data);
	}
	template<typename T>
	inline auto ShaderAttribute<T>::GetData() const -> const void*
	{
		return data.data();
	}
	template<typename T>
	inline auto ShaderAttribute<T>::GetSize() const -> size_t
	{
		return sizeof(T) * data.size();
	}
	template<typename T>
	inline auto ShaderAttribute<T>::GetStride() const -> size_t
	{
		return stride;
	}

	template<typename T>
	inline auto ShaderAttribute<T>::Clone() const -> std::unique_ptr<ShaderAttributeBase>
	{
		auto ptr = std::make_unique<ShaderAttribute<T>>(name, data);

		return ptr;
	}
}