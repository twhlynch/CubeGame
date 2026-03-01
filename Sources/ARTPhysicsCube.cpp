#include "ARTPhysicsCube.h"

#include "ARTTypes.h"
#include "ARTWorld.h"

namespace ART
{

static constexpr float scale = 0.06f;
static constexpr float mass = 1.0f;

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
}

RN::JoltDynamicBody *PhysicsCube::GetPhysicsBody() const
{
	return _physicsBody;
}

}; // namespace ART
