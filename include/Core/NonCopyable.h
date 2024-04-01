#pragma once

namespace sh::core {
	class NonCopyable {
	public:
		NonCopyable(){}
		NonCopyable(const NonCopyable& other) = delete;
		void operator=(const NonCopyable& other) = delete;
	};
}