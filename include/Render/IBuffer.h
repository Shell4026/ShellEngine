#pragma once

namespace sh::render
{
	class IBuffer
	{
	public:
		virtual ~IBuffer() = default;

		virtual void SetData(const void* data) = 0;
		virtual auto GetData() const -> void* = 0;
		virtual auto GetSize() const -> size_t = 0;
	};
}//namespace