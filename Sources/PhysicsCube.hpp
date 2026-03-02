#pragma once

#include <RNJoltDynamicBody.h>
#include <Rayne.h>

namespace ART
{

class PhysicsCube : public RN::Entity
{
public:
	PhysicsCube();

	RN::JoltDynamicBody *GetPhysicsBody() const;

private:
	static RN::Model *DefaultModel();

	RN::JoltDynamicBody *_physicsBody;
};

} // namespace ART
