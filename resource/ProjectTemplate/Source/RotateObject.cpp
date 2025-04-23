#include "RotateObject.h"

#include "Game/GameObject.h"
#include "Game/Vector.h"

using namespace sh::game;

RotateObject::RotateObject(GameObject& owner) :
	Component(owner),
	speed(30.f)
{
}

RotateObject::~RotateObject()
{
}

SH_USER_API void RotateObject::OnEnable()
{
}

SH_USER_API void RotateObject::Update()
{
	Vec3 rot = gameObject.transform->rotation;
	gameObject.transform->SetRotation(rot + Vec3{ xspeed * world.deltaTime, speed * world.deltaTime, zspeed * world.deltaTime });
}
