#pragma once

#include <RNJoltDynamicBody.h>
#include <RNJoltShape.h>
#include <Rayne.h>

#include "PhysicsObjects.hpp"

namespace CG
{

class PhysicsGroup : public RN::SceneNode
{
public:
	PhysicsGroup(PhysicsObject *object);
	~PhysicsGroup() override;

	void AddObject(PhysicsObject *object);
	void Merge(PhysicsGroup *other);

	RN::JoltDynamicBody *GetPhysicsBody();
	RN::Array *GetObjects();

	void StartManipulating();
	void StopManipulating();

	void Throw(RN::Vector3 linearVelocity, RN::Vector3 angularVelocity);
	void Grab();

	void CreatePhysicsBody();

private:
	void UpdateCollisionFilter(bool grabbing);

	RN::JoltCompoundShape *_shape;
	RN::JoltDynamicBody *_body;

	RN::Array *_objects;
	float _mass;
};

} // namespace CG
