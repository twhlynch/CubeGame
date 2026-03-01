#include "ARTPhysicsCube.h"

#include "ARTTypes.h"
#include "ARTWorld.h"

namespace ART
{

static constexpr float scale = 0.03f;
static constexpr float mass = 0.5f;
static constexpr float friction = 0.5f;

RN::Model *PhysicsCube::DefaultModel()
{
	static auto *model = World::GetSharedInstance()->AssignShader(RN::Model::WithCube(RN::Color::Red()), Types::MaterialType::MaterialDefault);
	return model;
}

PhysicsCube::PhysicsCube() : RN::Entity(DefaultModel())
{
	World *world = World::GetSharedInstance();

	static auto *material = new RN::JoltMaterial();
	static auto *shape = RN::JoltBoxShape::WithHalfExtents(scale, material->Autorelease());

	SetScale(scale);

	_physicsBody = RN::JoltDynamicBody::WithShape(shape, mass);
	_physicsBody->SetCollisionFilter(Types::CollisionGrabbable, Types::CollisionAll);
	AddAttachment(_physicsBody);

	_physicsBody->SetAllowSleeping(false);
	_physicsBody->SetEnableSleeping(false);
	_physicsBody->SetEnableGravity(true);
	_physicsBody->SetFriction(friction);
}

RN::JoltDynamicBody *PhysicsCube::GetPhysicsBody() const
{
	return _physicsBody;
}

}; // namespace ART
