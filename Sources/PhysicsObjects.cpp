#include "PhysicsObjects.hpp"

#include <RNJoltShape.h>

#include "ObjectManager.hpp"
#include "World.hpp"

namespace CG
{

constexpr float convexRadiusFactor = 0.001f;

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
	const auto worldScale = GetWorldScale();
	return RN::JoltBoxShape::WithHalfExtents(worldScale, worldScale.GetMin() * convexRadiusFactor);
}

RNDefineMeta(PhysicsSphere, PhysicsObject);

PhysicsSphere::PhysicsSphere(RN::Model *model) : PhysicsObject(model) {}

RN::JoltShape *PhysicsSphere::CreateShape() const
{
	const auto worldRadius = GetWorldScale().x;
	return RN::JoltSphereShape::WithRadius(worldRadius);
}

RNDefineMeta(PhysicsPyramid, PhysicsObject);

PhysicsPyramid::PhysicsPyramid(RN::Model *model) : PhysicsObject(model) {}

RN::JoltShape *PhysicsPyramid::CreateShape() const
{
	auto *mesh = World::GetSharedInstance()->GetObjectManager()->GetMeshWithIndex(2);
	const auto worldRadius = GetWorldScale().x;
	// RN::JoltTriangleMeshShape does not work due to having sharp edges with a convex radius of zero
	return RN::JoltConvexHullShape::WithMesh(mesh, worldRadius, worldRadius * convexRadiusFactor);
}

RNDefineMeta(PhysicsRectangularPrism, PhysicsObject);

PhysicsRectangularPrism::PhysicsRectangularPrism(RN::Model *model) : PhysicsObject(model) {}

RN::JoltShape *PhysicsRectangularPrism::CreateShape() const
{
	auto scale = GetWorldScale();
	scale.x *= 0.5f;
	scale.z *= 0.5f;
	return RN::JoltBoxShape::WithHalfExtents(scale, scale.GetMin() * convexRadiusFactor);
}

RNDefineMeta(PhysicsCylinder, PhysicsObject);

PhysicsCylinder::PhysicsCylinder(RN::Model *model) : PhysicsObject(model) {}

RN::JoltShape *PhysicsCylinder::CreateShape() const
{
	auto *mesh = World::GetSharedInstance()->GetObjectManager()->GetMeshWithIndex(4);
	const auto worldRadius = GetWorldScale().x;
	return RN::JoltConvexHullShape::WithMesh(mesh, worldRadius, worldRadius * convexRadiusFactor);
}

}; // namespace CG
