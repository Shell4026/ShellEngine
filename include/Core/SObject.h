#pragma once

#include "Export.h"

#include <vector>
#include <utility>

namespace sh::core
{
	class ISObject
	{
	private:
		//std::vector<std::pair<ISObject*>>
	public:
		SH_CORE_API virtual void Awake() = 0;
		SH_CORE_API virtual void Start() = 0;
		SH_CORE_API virtual void OnEnable() = 0;
		SH_CORE_API virtual void Update() = 0;
		SH_CORE_API virtual void LateUpdate() = 0;
	};
}