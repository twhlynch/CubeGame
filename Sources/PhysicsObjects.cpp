#include "PhysicsObjects.hpp"

#include <RNJoltMaterial.h>
#include <RNJoltShape.h>

#include "ObjectManager.hpp"
#include "World.hpp"

namespace CG
{

RNDefineMeta(PhysicsObject, RN::Entity);

PhysicsObject::PhysicsObject(RN::Model *model) : RN::Entity(model)
{
	static constexpr float initialScale = 0.03f;
	SetWorldScale(initialScale);
}

void PhysicsObject::Update(float delta)
{
	RN::Entity::Update(delta);

	constexpr float voidHeight = -10.0f; // 10m

	const RN::Vector3 worldPosition = GetWorldPosition();
	if (worldPosition.y < voidHeight)
	{
		RemoveFromParent();
	}
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

RNDefineMeta(PhysicsPyramid, PhysicsObject);

PhysicsPyramid::PhysicsPyramid(RN::Model *model) : PhysicsObject(model) {}

RN::JoltShape *PhysicsPyramid::CreateShape() const
{
	auto *material = new RN::JoltMaterial();
	auto *mesh = World::GetSharedInstance()->GetObjectManager()->GetMeshWithIndex(2);
	const auto worldRadius = GetWorldScale().x;
	// RN::JoltTriangleMeshShape does not work due to having sharp edges with a convex radius of zero
	return RN::JoltConvexHullShape::WithMesh(mesh, material->Autorelease(), worldRadius, worldRadius * 0.01f);
}

}; // namespace CG
