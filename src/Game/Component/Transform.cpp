#include "Component/Transform.h"

#include <iostream>

namespace sh::game
{
	Transform::Transform(GameObject& owner) :
		Component(owner)
	{

	}
	Transform::~Transform()
	{

	}

	void Transform::Awake()
	{
		Super::Awake();
		std::cout << "Transform Awake!\n";
	}

	void Transform::Start()
	{
		std::cout << "Transform Start!\n";
	}
}