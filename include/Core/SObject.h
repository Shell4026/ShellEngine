#pragma once

namespace sh::core
{
	class ISObject
	{
	public:
		virtual void Awake() = 0;
		virtual void Start() = 0;
		virtual void OnEnable() = 0;
		virtual void Update() = 0;
		virtual void LateUpdate() = 0;
	};
}