#include "PhysicsObjects.hpp"

#include <RNJoltMaterial.h>
#include <RNJoltShape.h>

namespace CG
{

RNDefineMeta(PhysicsObject, RN::Entity);

PhysicsObject::PhysicsObject(RN::Model *model) : RN::Entity(model)
{
	static constexpr float initialScale = 0.03f;
	SetWorldScale(initialScale);
}

RNDefineMeta(PhysicsCube, PhysicsObject);

PhysicsCube::PhysicsCube(RN::Model *model) : PhysicsObject(model) {}

RN::JoltShape *PhysicsCube::CreateShape() const
{
	auto *material = new RN::JoltMaterial();
	const auto worldScale = GetWorldScale();
	return RN::JoltBoxShape::WithHalfExtents(worldScale, material->Autorelease(), worldScale.GetMin());
}

RNDefineMeta(PhysicsSphere, PhysicsObject);

PhysicsSphere::PhysicsSphere(RN::Model *model) : PhysicsObject(model) {}

RN::JoltShape *PhysicsSphere::CreateShape() const
{
	auto *material = new RN::JoltMaterial();
	const auto worldRadius = GetWorldScale().x;
	return RN::JoltSphereShape::WithRadius(worldRadius, material->Autorelease());
}

}; // namespace CG
