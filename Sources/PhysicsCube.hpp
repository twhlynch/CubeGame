#pragma once

#include <RNJoltDynamicBody.h>
#include <Rayne.h>

namespace ART
{

class PhysicsCube : public RN::Entity
{
public:
	PhysicsCube(RN::Model *model);

	RN::JoltDynamicBody *GetPhysicsBody() const;

private:
	static RN::Model *DefaultModel(const RN::Color &color);

	RN::JoltDynamicBody *_physicsBody;
};

} // namespace ART
