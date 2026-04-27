#pragma once

#include <vector>
namespace sh::render
{
	class IBuffer
	{
	public:
		virtual ~IBuffer() = default;

		virtual void SetData(const void* data) = 0;
		virtual void SetData(const void* data, std::size_t offset, std::size_t size) = 0;
		virtual auto GetData() const -> std::vector<uint8_t> = 0;
		virtual auto GetSize() const -> size_t = 0;
		virtual auto Resize(std::size_t size) -> bool = 0;
	};
}//namespace