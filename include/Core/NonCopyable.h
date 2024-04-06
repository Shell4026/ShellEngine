#pragma once

namespace sh::core {
	class INonCopyable {
	public:
		INonCopyable() = default;
		INonCopyable(const INonCopyable& other) = delete;
		void operator=(const INonCopyable& other) = delete;
	};
}