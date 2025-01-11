#pragma once
#include "Export.h"
#include "Shader.h"
#include "Texture.h"
#include "IBuffer.h"
#include "IRenderResource.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"
#include "Core/SContainer.hpp"
#include "Core/ISyncable.h"

#include "glm/mat4x4.hpp"

#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>

namespace sh::render
{
	class Renderer;
	class IUniformBuffer;

	class Material : 
		public sh::core::SObject, 
		public IRenderResource, 
		public core::ISyncable
	{
		SCLASS(Material)
	private:
		struct PassData
		{
			core::SMap<uint32_t, std::unique_ptr<IBuffer>> vertShaderData;
			core::SMap<uint32_t, std::unique_ptr<IBuffer>> fragShaderData;
			std::unique_ptr<IUniformBuffer> uniformBuffer; // 메테리얼 공통 유니폼 버퍼
		};
	private:
		const IRenderContext* context = nullptr;

		PROPERTY(shader)
		Shader* shader;

		core::SyncArray<std::vector<PassData>> perPassData;

		PROPERTY(floats)
		core::SMap<std::string, float, 4> floats;
		PROPERTY(ints)
		core::SMap<std::string, int, 4> ints;
		core::SMap<std::string, glm::mat4, 4> mats;
		PROPERTY(vectors)
		core::SMap<std::string, glm::vec4, 4> vectors;
		std::unique_ptr<core::SMap<std::string, std::vector<float>, 4>> floatArr;
		core::SMap<std::string, std::vector<glm::vec4>, 4> vectorArrs;
		PROPERTY(textures)
		core::SMap<std::string, Texture*, 4> textures;
	public:
		bool bDirty, bBufferDirty, bBufferSync;
	private:
		void Clean();
		/// @brief [게임 스레드용] 유니폼에 데이터를 지정한다.
		/// @param pass 패스 번호
		/// @param binding 바인딩 번호
		/// @param data 데이터 위치 포인터
		/// @param stage 셰이더 스테이지
		SH_RENDER_API void SetUniformData(std::size_t passIdx, uint32_t binding, const void* data, ShaderPass::ShaderStage stage);
		/// @brief [게임 스레드용] 유니폼 텍스쳐 데이터를 지정한다.
		/// /// @param pass 패스 번호
		/// @param binding 텍스쳐 바인딩 번호
		/// @param tex 텍스쳐 포인터
		SH_RENDER_API void SetTextureData(std::size_t passIdx, uint32_t binding, Texture* tex);

		template<typename T>
		void SetData(const T& data, std::vector<uint8_t>& uniformData, std::size_t offset, std::size_t size)
		{
			std::memcpy(uniformData.data() + offset, &data, size);
		}
		template<typename T>
		void SetData(const T& data, std::vector<uint8_t>& uniformData, std::size_t offset)
		{
			std::memcpy(uniformData.data() + offset, &data, sizeof(T));
		}
		void FillData(const ShaderPass::UniformBlock& uniformBlock, std::vector<uint8_t>& dst, uint32_t binding);
		void SetDefaultProperties();
	public:
		SH_RENDER_API Material();
		SH_RENDER_API Material(Shader* shader);
		SH_RENDER_API ~Material();
		SH_RENDER_API Material(Material&& other) noexcept;

		SH_RENDER_API void SetShader(Shader* shader);
		SH_RENDER_API auto GetShader() const -> Shader*;
		
		/// @brief 렌더러API에 맞는 버퍼와 유니폼 버퍼를 생성한다.
		/// @param renderer 렌더러
		SH_RENDER_API void Build(const IRenderContext& context) override;

		SH_RENDER_API void SetDirty() override;
		SH_RENDER_API void Sync() override;

		SH_RENDER_API auto GetShaderBuffer(std::size_t passIdx, ShaderPass::ShaderStage stage, uint32_t binding, core::ThreadType thr = core::ThreadType::Game) const -> IBuffer*;
		SH_RENDER_API auto GetUniformBuffer(std::size_t passIdx, core::ThreadType thr = core::ThreadType::Game) const -> IUniformBuffer*;

		/// @brief 유니폼 버퍼를 갱신하는 함수. 변화가 있던 경우에만 갱신한다.
		SH_RENDER_API void UpdateUniformBuffers();

		SH_RENDER_API void SetInt(const std::string& name, int value);
		SH_RENDER_API auto GetInt(const std::string& name) const -> int;
		SH_RENDER_API void SetFloat(const std::string& name, float value);
		SH_RENDER_API auto GetFloat(const std::string& name) const -> float;
		SH_RENDER_API void SetVector(const std::string& name, const glm::vec4& value);
		SH_RENDER_API auto GetVector(const std::string& name) const -> const glm::vec4*;
		SH_RENDER_API void SetMatrix(const std::string& name, const glm::mat4& value);
		SH_RENDER_API auto GetMatrix(const std::string& name) const -> const glm::mat4*;
		SH_RENDER_API void SetFloatArray(const std::string& name, const std::vector<float>& value);
		SH_RENDER_API void SetFloatArray(const std::string& name, std::vector<float>&& value);
		SH_RENDER_API auto GetFloatArray(const std::string& name) const -> const std::vector<float>*;
		SH_RENDER_API void SetVectorArray(const std::string& name, const std::vector<glm::vec4>& value);
		SH_RENDER_API auto GetVectorArray(const std::string& name) const -> const std::vector<glm::vec4>*;
		SH_RENDER_API void SetTexture(const std::string& name, Texture* tex);
		SH_RENDER_API auto GetTexture(const std::string& name) const -> Texture*;
		
		SH_RENDER_API bool HasIntProperty(const std::string& name) const;
		SH_RENDER_API bool HasFloatProperty(const std::string& name) const;
		SH_RENDER_API bool HasVectorProperty(const std::string& name) const;
		SH_RENDER_API bool HasMatrixProperty(const std::string& name) const;
		SH_RENDER_API bool HasFloatArrayProperty(const std::string& name) const;
		SH_RENDER_API bool HasVectorArrayProperty(const std::string& name) const;
		SH_RENDER_API bool HasTextureProperty(const std::string& name) const;

		SH_RENDER_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_RENDER_API auto Serialize() const -> core::Json override;
		SH_RENDER_API void Deserialize(const core::Json& json) override;
	};
}