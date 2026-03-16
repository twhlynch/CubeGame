#include "PhysicsGroup.hpp"

#include <Math/RNVector.h>

#include "PhysicsObjects.hpp"
#include "Types.hpp"

namespace CG
{

static constexpr float mass = 0.5f;
static constexpr float friction = 0.5f;

PhysicsGroup::PhysicsGroup(PhysicsObject *object) : _mass(0), _body(nullptr)
{
	_objects = new RN::Array();
	_shape = new RN::JoltCompoundShape();

	AddObject(object);

	CreatePhysicsBody();
}

PhysicsGroup::~PhysicsGroup()
{
	_objects->Release();
	_shape->Release();
}

void PhysicsGroup::UpdateCollisionFilter(bool grabbing)
{
	const RN::uint32 group = grabbing ? Types::CollisionGrabbing : Types::CollisionGrabbable;
	const RN::uint32 mask = grabbing
								? Types::CollisionLevel | Types::CollisionGrabbable | Types::CollisionTest
								: Types::CollisionLevel | Types::CollisionGrabbable | Types::CollisionTest | Types::CollisionGrabbing;

	_body->SetCollisionFilter(group, mask);
}

void PhysicsGroup::AddObject(PhysicsObject *object)
{
	const RN::Vector3 worldPos = object->GetWorldPosition();
	const RN::Quaternion worldRot = object->GetWorldRotation();
	const RN::Vector3 worldSca = object->GetWorldScale();

	const RN::Vector3 parentPos = GetWorldPosition();
	const RN::Quaternion parentRot = GetWorldRotation();
	const RN::Vector3 parentSca = GetWorldScale();

	const RN::Quaternion invRot = parentRot.GetInverse();

	const RN::Vector3 localPos = invRot.GetRotatedVector(worldPos - parentPos) / parentSca;
	const RN::Quaternion localRot = invRot * worldRot;
	const RN::Vector3 localSca = worldSca / parentSca;

	_objects->AddObject(object);
	_shape->AddChild(object->CreateShape(), localPos, localRot);

	_mass += mass;
	if (_body) { _body->SetMass(_mass); }

	object->RemoveFromParent();
	AddChild(object);

	object->SetPosition(localPos);
	object->SetRotation(localRot);
	object->SetScale(localSca);
}

void PhysicsGroup::Merge(PhysicsGroup *other)
{
	if (!other || other == this) { return; }

	other->GetObjects()->Enumerate<PhysicsObject>([&](PhysicsObject *object, size_t, bool &) {
		AddObject(object);
	});

	// remove other object
	other->RemoveFromParent();

	CreatePhysicsBody();

	UpdateCollisionFilter(true);

	_body->SetEnableKinematic(true);
	_body->SetEnableGravity(false);
}

void PhysicsGroup::CreatePhysicsBody()
{
	if (_body) { RemoveAttachment(_body); };

	_shape->Release();
	_shape = new RN::JoltCompoundShape();

	const RN::Vector3 scale = GetScale();

	_objects->Enumerate<PhysicsObject>([&](PhysicsObject *object, size_t, bool &) {
		_shape->AddChild(object->CreateShape(), object->GetPosition() * scale, object->GetRotation());
	});

	_body = RN::JoltDynamicBody::WithShape(_shape, _mass);

	UpdateCollisionFilter(false);

	AddAttachment(_body);

	_body->SetAllowSleeping(false);
	_body->SetEnableSleeping(false);
	_body->SetEnableGravity(true);
	_body->SetFriction(friction);
}

void PhysicsGroup::StartManipulating()
{
	if (!_body) { return; }

	RemoveAttachment(_body);
	_body = nullptr;
}

void PhysicsGroup::StopManipulating()
{
	// recreate physics body with new scale
	CreatePhysicsBody();

	UpdateCollisionFilter(true);

	_body->SetEnableKinematic(true);
	_body->SetEnableGravity(false);
}

void PhysicsGroup::Throw(RN::Vector3 linearVelocity, RN::Vector3 angularVelocity)
{
	// enable collision with grabbed objects
	UpdateCollisionFilter(false);

	// enable physics
	_body->SetEnableKinematic(false);
	_body->SetEnableGravity(true);

	// move with velocity
	_body->SetLinearVelocity(linearVelocity);
	_body->SetAngularVelocity(angularVelocity);
}

void PhysicsGroup::Grab()
{
	// disable collision with grabbed objects
	UpdateCollisionFilter(true);

	// disable physics
	_body->SetEnableKinematic(true);
	_body->SetEnableGravity(false);
}

RN::Array *PhysicsGroup::GetObjects()
{
	return _objects;
}

RN::JoltDynamicBody *PhysicsGroup::GetPhysicsBody()
{
	return _body;
}

} // namespace CG
