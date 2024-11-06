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
		const Renderer* renderer;

		PROPERTY(shader)
		Shader* shader;

		core::SyncArray<core::SMap<uint32_t, std::unique_ptr<IBuffer>>> vertBuffers;
		core::SyncArray<core::SMap<uint32_t, std::unique_ptr<IBuffer>>> fragBuffers;
		core::SyncArray<std::unique_ptr<IUniformBuffer>> uniformBuffer; // 메테리얼 공통 유니폼 버퍼

		core::SMap<std::string, float, 4> floats;
		core::SMap<std::string, int, 4> ints;
		core::SMap<std::string, glm::mat4, 4> mats;
		core::SMap<std::string, glm::vec4, 4> vectors;
		std::unique_ptr<core::SMap<std::string, std::vector<float>, 4>> floatArr;
		core::SMap<std::string, std::vector<glm::vec4>, 4> vectorArrs;
		PROPERTY(textures)
		core::SMap<uint32_t, Texture*, 4> textures;

		
	public:
		bool bDirty, bBufferDirty, bBufferSync;
		enum class Stage
		{
			Vertex,
			Fragment
		};
	private:
		void Clean();
		/// @brief 유니폼에 데이터를 지정한다.
		/// @param binding 바인딩 번호
		/// @param data 데이터 위치 포인터
		/// @param stage 셰이더 스테이지
		SH_RENDER_API void SetUniformData(uint32_t binding, const void* data, Stage stage);
		/// @brief 유니폼 텍스쳐 데이터를 지정한다.
		/// @param binding 텍스쳐 바인딩 번호
		/// @param tex 텍스쳐 포인터
		SH_RENDER_API void SetTextureData(uint32_t binding, Texture* tex);

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
		void FillData(const Shader::UniformBlock& uniformBlock, std::vector<uint8_t>& dst, uint32_t binding);
	public:
		SH_RENDER_API Material();
		SH_RENDER_API Material(Shader* shader);
		SH_RENDER_API ~Material();
		SH_RENDER_API Material(Material&& other) noexcept;

		SH_RENDER_API void SetShader(Shader* shader);
		SH_RENDER_API auto GetShader() const->Shader*;
		
		/// @brief 렌더러API에 맞는 버퍼와 유니폼 버퍼를 생성한다.
		/// @param renderer 렌더러
		SH_RENDER_API void Build(const Renderer& renderer) override;

		SH_RENDER_API void SetDirty() override;
		SH_RENDER_API void Sync() override;

		SH_RENDER_API auto GetShaderBuffer(Stage stage, uint32_t binding, core::ThreadType thr = core::ThreadType::Game) const -> IBuffer*;
		SH_RENDER_API auto GetUniformBuffer(core::ThreadType thr = core::ThreadType::Game) const -> IUniformBuffer*;

		/// @brief 유니폼 버퍼를 갱신하는 함수. 변화가 있던 경우에만 갱신한다.
		SH_RENDER_API void UpdateUniformBuffers();

		SH_RENDER_API void SetInt(std::string_view name, int value);
		SH_RENDER_API auto GetInt(std::string_view name) const -> int;
		SH_RENDER_API void SetFloat(std::string_view name, float value);
		SH_RENDER_API auto GetFloat(std::string_view name) const -> float;
		SH_RENDER_API void SetVector(std::string_view name, const glm::vec4& value);
		SH_RENDER_API auto GetVector(std::string_view name) const -> const glm::vec4*;
		SH_RENDER_API void SetMatrix(std::string_view name, const glm::mat4& value);
		SH_RENDER_API auto GetMatrix(std::string_view name) const -> const glm::mat4*;
		SH_RENDER_API void SetFloatArray(std::string_view name, const std::vector<float>& value);
		SH_RENDER_API auto GetFloatArray(std::string_view name) const -> const std::vector<float>*;
		SH_RENDER_API void SetVectorArray(std::string_view name, const std::vector<glm::vec4>& value);
		SH_RENDER_API auto GetVectorArray(std::string_view name) const -> const std::vector<glm::vec4>*;
		SH_RENDER_API void SetTexture(std::string_view name, Texture* tex);
		SH_RENDER_API auto GetTexture(std::string_view name) const -> Texture*;
	};
}