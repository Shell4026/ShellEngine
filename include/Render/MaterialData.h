#pragma once
#include "Export.h"
#include "IBuffer.h"
#include "IUniformBuffer.h"
#include "ShaderEnum.h"

#include "Core/ISyncable.h"
#include "Core/SContainer.hpp"

#include <map>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <vector>
#include <variant>
namespace sh::render
{
	class Shader;
	class ShaderPass;
	class IRenderContext;

	/// @brief 셰이더에 전달 할 데이터를 가지고 있는 클래스
	/// @brief 모든 변경 사항은 게임 스레드에서만 작성 시 스레드 안전하다.
	class MaterialData : 
		public core::ISyncable, 
		public core::INonCopyable
	{
	private:
		struct PassData
		{
			const ShaderPass* pass;
			std::map<uint32_t, std::vector<std::unique_ptr<IBuffer>>> uniformData; //set, binding
			std::map<uint32_t, std::unique_ptr<IUniformBuffer>> uniformBuffer; // set
		};
		struct SyncData
		{
			struct BufferSyncData
			{
				const ShaderPass* pass;
				uint32_t set;
				uint32_t binding;
				std::vector<uint8_t> data;
			};
			struct UniformBufferSyncData
			{
				const ShaderPass* pass;
				uint32_t set;
				uint32_t binding;
				const Texture* tex = nullptr;
			};
			std::variant<BufferSyncData, UniformBufferSyncData> data;
		};
	public:
		SH_RENDER_API MaterialData() = default;
		SH_RENDER_API MaterialData(MaterialData&& other) noexcept;

		SH_RENDER_API void Create(const IRenderContext& context, const Shader& shader, bool bPerObject = false);
		SH_RENDER_API void Clear();
		/// @brief 유니폼에 데이터를 지정한다.
		/// @param pass 패스 번호
		/// @param binding 바인딩 번호
		/// @param data 데이터 위치 포인터
		/// @param dataSize 데이터 사이즈
		SH_RENDER_API void SetUniformData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, const void* data, std::size_t dataSize);
		/// @brief 유니폼에 데이터를 지정한다.
		/// @param pass 패스 번호
		/// @param binding 바인딩 번호
		/// @param data 데이터
		SH_RENDER_API void SetUniformData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, const std::vector<uint8_t>& data);
		/// @brief 유니폼에 데이터를 지정한다.
		/// @param pass 패스 번호
		/// @param binding 바인딩 번호
		/// @param data 데이터
		SH_RENDER_API void SetUniformData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, std::vector<uint8_t>&& data);
		/// @brief 유니폼 텍스쳐 데이터를 지정한다.
		/// /// @param pass 패스 번호
		/// @param binding 텍스쳐 바인딩 번호
		/// @param tex 텍스쳐
		SH_RENDER_API void SetTextureData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, const Texture& tex);

		SH_RENDER_API auto GetShaderBuffer(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding) const -> IBuffer*;
		SH_RENDER_API auto GetUniformBuffer(const ShaderPass& shaderPass, UniformStructLayout::Type type) const -> IUniformBuffer*;

		SH_RENDER_API void SyncDirty() override;
	protected:
		SH_RENDER_API void Sync() override;
	private:
		void CreateBuffers(const IRenderContext& context, const Shader& shader, bool bPerObject);
		auto GetMaterialPassData(const ShaderPass& shaderPass) const -> const MaterialData::PassData*;
		void SetUniformDataAtSync(const SyncData::BufferSyncData& bufferSyncData);
		void SetTextureDataAtSync(const SyncData::UniformBufferSyncData& uniformBufferSyncData);
	private:
		const IRenderContext* context = nullptr;
		const Shader* shader = nullptr;

		std::unordered_map<const ShaderPass*, PassData> perPassData;

		std::vector<SyncData> syncDatas;

		bool bDirty = false;
		bool bClearDirty = false;
		bool bCreateDirty = false;
		bool bPerObject = false;
	};
}//namespace