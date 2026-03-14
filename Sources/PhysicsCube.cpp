#include "PhysicsCube.hpp"

#include <RNJoltMaterial.h>
#include <RNJoltShape.h>

#include "Types.hpp"

namespace CG
{

static constexpr float scale = 0.03f;
static constexpr float mass = 0.5f;
static constexpr float friction = 0.5f;

PhysicsCube::PhysicsCube(RN::Model *model) : RN::Entity(model)
{
	SetScale(scale);
	CreatePhysicsBody();
}

void PhysicsCube::CreatePhysicsBody()
{
	auto *material = new RN::JoltMaterial();
	auto *shape = RN::JoltBoxShape::WithHalfExtents(GetScale(), material->Autorelease(), GetScale().GetMin());

	_physicsBody = RN::JoltDynamicBody::WithShape(shape, mass);
	_physicsBody->SetCollisionFilter(Types::CollisionGrabbable, Types::CollisionLevel | Types::CollisionGrabbable | Types::CollisionTest);

	AddAttachment(_physicsBody);

	_physicsBody->SetAllowSleeping(false);
	_physicsBody->SetEnableSleeping(false);
	_physicsBody->SetEnableGravity(true);
	_physicsBody->SetFriction(friction);
}

void PhysicsCube::StartManipulating()
{
	RemoveAttachment(_physicsBody);
	_physicsBody = nullptr;
}

void PhysicsCube::StopManipulating()
{
	CreatePhysicsBody();
	_physicsBody->SetEnableKinematic(true);
	_physicsBody->SetEnableGravity(false);
}

void PhysicsCube::Throw(RN::Vector3 linearVelocity, RN::Vector3 angularVelocity)
{
	// enable physics
	_physicsBody->SetEnableKinematic(false);
	_physicsBody->SetEnableGravity(true);

	// move with velocity
	_physicsBody->SetLinearVelocity(linearVelocity);
	_physicsBody->SetAngularVelocity(angularVelocity);
}

void PhysicsCube::Grab()
{
	_physicsBody->SetEnableKinematic(true);
	_physicsBody->SetEnableGravity(false);
}

RN::JoltDynamicBody *PhysicsCube::GetPhysicsBody() const
{
	return _physicsBody;
}

}; // namespace CG
