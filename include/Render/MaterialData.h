#pragma once
#include "Export.h"
#include "IBuffer.h"
#include "IUniformBuffer.h"
#include "ShaderEnum.h"

#include "Core/ISyncable.h"

#include <map>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <vector>

namespace sh::render
{
	class Shader;
	class ShaderPass;
	class IRenderContext;

	/// @brief 셰이더에 전달 할 데이터를 가지고 있는 클래스
	class MaterialData : 
		public core::ISyncable, 
		public core::INonCopyable
	{
	private:
		const IRenderContext* context = nullptr;
		const Shader* shader = nullptr;

		struct PassData
		{
			// uint32_t = binding
			const ShaderPass* pass;
			core::SVector<core::SVector<std::unique_ptr<IBuffer>>> uniformData; // set, binding
			core::SVector<std::unique_ptr<IUniformBuffer>> uniformBuffer; // set
		};
		core::SyncArray<core::SVector<PassData>> perPassData;

		bool bPerObject = false;
		bool bDirty = false;
		bool bRecreate = false;
	private:
		void CreateBuffers(core::ThreadType thread, const IRenderContext& context, const Shader& shader, bool bPerObject);
	protected:
		SH_RENDER_API void SetDirty() override;
		SH_RENDER_API void Sync() override;
	public:
		SH_RENDER_API MaterialData() = default;
		SH_RENDER_API MaterialData(MaterialData&& other) noexcept;

		SH_RENDER_API void Create(const IRenderContext& context, const Shader& shader, bool bPerObject = false);
		SH_RENDER_API void Clear();
		/// @brief 유니폼에 데이터를 지정한다.
		/// @param pass 패스 번호
		/// @param binding 바인딩 번호
		/// @param data 데이터 위치 포인터
		/// @param stage 셰이더 스테이지
		SH_RENDER_API void SetUniformData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, const void* data, core::ThreadType thread);
		/// @brief [게임 스레드용] 유니폼 텍스쳐 데이터를 지정한다.
		/// /// @param pass 패스 번호
		/// @param binding 텍스쳐 바인딩 번호
		/// @param tex 텍스쳐 포인터
		SH_RENDER_API void SetTextureData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, const Texture* tex);

		SH_RENDER_API auto GetShaderBuffer(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, 
			core::ThreadType thr = core::ThreadType::Game) const -> IBuffer*;
		SH_RENDER_API auto GetUniformBuffer(const ShaderPass& shaderPass, UniformStructLayout::Type type, core::ThreadType thr = core::ThreadType::Game) const -> IUniformBuffer*;
	};
}//namespace