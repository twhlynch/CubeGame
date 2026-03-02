#pragma once

#include <RNJoltShape.h>
#include <Rayne.h>

#include "PhysicsCube.hpp"

namespace ART
{

class Hand : public RN::SceneNode
{
public:
	Hand(uint8_t index);
	~Hand() override;

	void Update(float delta) override;

private:
	void UpdateFingers(float delta);
	void UpdateLeftHand(float delta);
	void UpdateRightHand(float delta);

	void TryGrabObject();
	void GrabObject(PhysicsCube *object);
	void DropObject();

	uint8_t _handIndex;

	std::array<RN::Entity *, 6> _indicator;
	std::array<bool, 4> _pinching;
	std::array<bool, 4> _wasPinching;

	RN::JoltSphereShape *_intersectShape;

	RN::Vector3 _previousPosition;
	RN::Quaternion _previousRotation;
	RN::Vector3 _linearVelocity;
	RN::Vector3 _angularVelocity;
	RN::Vector3 _grabPositionOffset;
	RN::Quaternion _grabRotationOffset;

	PhysicsCube *_grabbedObject;
};

} // namespace ART
