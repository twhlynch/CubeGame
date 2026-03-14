#pragma once

#include <RNJoltShape.h>
#include <Rayne.h>

#include "PartsPicker.hpp"
#include "PhysicsCube.hpp"

namespace CG
{

class Hand : public RN::SceneNode
{
public:
	Hand(uint8_t index);
	~Hand() override;

	void Update(float delta) override;

	PartsPicker *GetPartsPicker() { return _partsPicker; }
	PhysicsCube *GetGrabbedObject() { return _grabbedObject; }
	bool GetHasStartedTracking() const { return _hasStartedTracking; }

private:
	void UpdateFingers(float delta);
	void UpdateInteractions(float delta);
	void UpdatePartsPicker(float delta);
	void UpdateScalingObject(float delta);
	void UpdateMovingObject(float delta);

	void TryGrabObject();
	void GrabObject(PhysicsCube *object);
	void DropObject();

	uint8_t _handIndex;
	bool _hasStartedTracking;

	std::array<RN::Entity *, 6> _indicator;
	std::array<bool, 4> _pinching;
	std::array<bool, 4> _wasPinching;

	RN::JoltSphereShape *_intersectShape;

	// for throwing
	RN::Vector3 _previousPosition;
	RN::Quaternion _previousRotation;
	RN::Vector3 _linearVelocity;
	RN::Vector3 _angularVelocity;
	RN::Vector3 _grabPositionOffset;
	RN::Quaternion _grabRotationOffset;

	// for scaling
	bool _scaling;

	float _initialHandDistance;
	RN::Vector3 _initialObjectScale;
	RN::Vector3 _initialGrabLocal0;
	RN::Vector3 _initialGrabLocal1;

	PhysicsCube *_grabbedObject;
	PartsPicker *_partsPicker;
};

} // namespace CG
