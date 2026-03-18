#pragma once

#include <Math/RNMatrixQuaternion.h>
#include <Scene/RNSceneNode.h>

#include <RNJoltCollisionObject.h>
#include <RNJoltShape.h>
#include <RNVRTrackingState.h>
#include <Rayne.h>
#include <RayneConfig.h>

#include "PartsPicker.hpp"
#include "PhysicsGroup.hpp"

namespace CG
{

using Joint = RN::VRHandTrackingState::Joint;

enum Pinch : RN::uint8
{
	Index,
	Middle,
	Ring,
	Little,
	_Count, // NOLINT(bugprone-reserved-identifier)
};

class Hand : public RN::SceneNode
{

public:
	Hand(uint8_t index);
	~Hand() override;

	void Update(float delta) override;

	PartsPicker *GetPartsPicker() { return _partsPicker; }
	PhysicsGroup *GetGrabbedObject() { return _grabbedObject; }
	bool GetHasStartedTracking() const { return _hasStartedTracking; }

private:
	void UpdateFingers(float delta);
	void UpdateInteractions(float delta);
	void UpdatePartsPicker(float delta);
	void UpdateScalingObject(float delta);
	void UpdateMovingObject(float delta);

	void TryGrabObject();
	void GrabObject(PhysicsGroup *object);
	void DropObject();

	template <class T>
	T *GetPinchOverlap(RN::uint32 mask);

	bool IsMainHand() const;
	Hand *GetOtherHand() const;
	RN::Vector3 GetPinchTarget() const;
	RN::Quaternion GetPinchRotation() const;

	bool IsPinching(size_t pinch) const;
	bool WasPinching(size_t pinch) const;
	RN::Entity *GetJointIndicator(size_t joint) const;

	uint8_t _handIndex;
	bool _hasStartedTracking;

	std::array<RN::Entity *, Joint::_JointCount> _indicator;

	std::array<bool, Pinch::_Count> _pinching;
	std::array<bool, Pinch::_Count> _wasPinching;

	RN::JoltSphereShape *_intersectShape;

	PhysicsGroup *_grabbedObject;
	PartsPicker *_partsPicker;

	// for throwing
	RN::Vector3 _previousPosition;
	RN::Quaternion _previousRotation;
	RN::Vector3 _linearVelocity;
	RN::Vector3 _angularVelocity;
	RN::Vector3 _grabPositionOffset;
	RN::Quaternion _grabRotationOffset;

	// data for scaling only needs to exist
	// once since it requires both hands
	static bool _scaling;
	static float _initialHandDistance;
	static RN::Vector3 _initialObjectScale;
	static RN::Vector3 _initialGrabLocal0;
	static RN::Vector3 _initialGrabLocal1;
};

} // namespace CG
