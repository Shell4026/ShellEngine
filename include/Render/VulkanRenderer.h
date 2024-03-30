#pragma once

#include "Renderer.h"

namespace sh::render {
	class VulkanRenderer : public Renderer {
	public:
		VulkanRenderer();
		~VulkanRenderer();

		void Init() override;
	};
}//namespace
