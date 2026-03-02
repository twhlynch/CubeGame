#pragma once

#include <Math/RNVector.h>

#include <RNJoltDynamicBody.h>
#include <Rayne.h>

namespace ART
{

class PhysicsCube : public RN::Entity
{
public:
	PhysicsCube(RN::Model *model);

	RN::JoltDynamicBody *GetPhysicsBody() const;

	void StartManipulating();
	void StopManipulating();

	void Throw(RN::Vector3 linearVelocity, RN::Vector3 angularVelocity);
	void Grab();

private:
	void CreatePhysicsBody();

	RN::JoltDynamicBody *_physicsBody;
};

} // namespace ART
