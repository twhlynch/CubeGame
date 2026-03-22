#include "Hand.hpp"

#include <Math/RNMatrixQuaternion.h>
#include <Math/RNVector.h>

#include <RNVRTrackingState.h>
#include <RayneConfig.h>

#include "Part.hpp"
#include "PartsPicker.hpp"
#include "PhysicsGroup.hpp"
#include "PhysicsHelpers.hpp"
#include "Types.hpp"
#include "World.hpp"

namespace CG
{

bool Hand::_scaling{};
float Hand::_initialHandDistance{};
RN::Vector3 Hand::_initialObjectScale{};
RN::Vector3 Hand::_initialGrabLocal0{};
RN::Vector3 Hand::_initialGrabLocal1{};

Hand::Hand(uint8_t index)
	: _handIndex(index), _hasStartedTracking(false),
	  _grabbedObject(nullptr),
	  _pinching({false}), _wasPinching({false})
{
	World *world = World::GetSharedInstance();

	// small cubes to indicate hand location
	auto *model = world->AssignShader(RN::Model::WithCube(RN::Color::Blue()), Types::MaterialType::MaterialDefault);

	for (size_t i = 0; i < Joint::_JointCount; i++)
	{
		auto *indicator = new RN::Entity(model);
		indicator->SetScale(0.001f);
		_indicator.at(i) = indicator;
		AddChild(indicator->Autorelease());
	}

	_intersectShape = RN::JoltSphereShape::WithRadius(0.02f, nullptr);
	_intersectShape->Retain();

	// parts picker
	_partsPicker = new PartsPicker();
	auto *palm = GetJointIndicator(Joint::Palm);
	palm->AddChild(_partsPicker->Autorelease());
	_partsPicker->SetWorldScale(1);
	_partsPicker->SetRotation(RN::Vector3(0, -20, 0));
}

Hand::~Hand()
{
	SafeRelease(_intersectShape);
	SafeRelease(_grabbedObject);
}

void Hand::Update(float delta)
{
	World *world = World::GetSharedInstance();
	RN::VRCamera *vrCamera = world->GetVRCamera();
	if (!vrCamera) { return; }

	delta = std::max(delta, RN::k::EpsilonFloat);

	// update position to camera
	SetWorldPosition(vrCamera->GetWorldPosition());

	UpdateFingers(delta);
	UpdateInteractions(delta);
	UpdatePartsPicker(delta);
}

void Hand::UpdateInteractions(float delta)
{
	if (IsPinching(Pinch::Index))
	{
		// pinch to grab
		if (!_grabbedObject && !WasPinching(Pinch::Index))
		{
			TryGrabObject();
		}

		// object interactions
		if (_grabbedObject)
		{
			if (_scaling)
			{
				UpdateScalingObject(delta);
			}
			else
			{
				UpdateMovingObject(delta);
			}
		}
	}
	else
	{
		DropObject();
	}

	const RN::Vector3 pinch = GetPinchTarget();
	const RN::Quaternion rotation = GetPinchRotation();

	// accumulate velocity
	const RN::Vector3 linearVelocity = (pinch - _previousPosition) / delta;
	const RN::Vector3 angularVelocity = _previousRotation.GetAngularVelocity(rotation, delta);

	_linearVelocity = _linearVelocity.GetLerp(linearVelocity, 0.9f);
	_angularVelocity = _angularVelocity.GetLerp(angularVelocity, 0.9f);

	// next frame info
	_previousPosition = pinch;
	_previousRotation = rotation;
}

void Hand::UpdateMovingObject(float delta)
{
	const RN::Vector3 handPosition = GetPinchTarget();
	const RN::Quaternion handRotation = GetPinchRotation();

	const RN::Vector3 worldOffset = handRotation.GetRotatedVector(_grabPositionOffset);

	const RN::Vector3 position = handPosition + worldOffset;
	const RN::Quaternion rotation = (handRotation * _grabRotationOffset).Normalize();

	_grabbedObject->GetPhysicsBody()->SetKinematicTarget(position, rotation, delta);
}

void Hand::UpdateScalingObject(float /*delta*/)
{
	if (!IsMainHand()) { return; } // only do scaling logic once

	World *world = World::GetSharedInstance();

	const RN::Vector3 p0 = GetPinchTarget();
	const RN::Vector3 p1 = GetOtherHand()->GetPinchTarget();

	// scale
	const float currentDistance = (p1 - p0).GetLength();
	const float scaleFactor = currentDistance / _initialHandDistance;

	const RN::Vector3 scale = _initialObjectScale * scaleFactor;

	// scaled local offsets
	const RN::Vector3 scaledLocal0 = _initialGrabLocal0 * scaleFactor;
	const RN::Vector3 scaledLocal1 = _initialGrabLocal1 * scaleFactor;

	const RN::Quaternion objectRotation = _grabbedObject->GetWorldRotation();
	const RN::Vector3 worldOffset0 = objectRotation.GetRotatedVector(scaledLocal0);
	const RN::Vector3 worldOffset1 = objectRotation.GetRotatedVector(scaledLocal1);

	// position between anchors
	const RN::Vector3 position = ((p0 - worldOffset0) + (p1 - worldOffset1)) / 2.0f;

	// apply
	_grabbedObject->SetWorldPosition(position);
	_grabbedObject->SetScale(scale);
}

void Hand::UpdatePartsPicker(float /*delta*/)
{
	// use the palm as the main rotation of the hand
	// the parts picker will float above it
	auto *palm = GetJointIndicator(Joint::Palm);
	const auto handRotation = palm->GetWorldEulerAngle();

	bool otherPickerHidden = GetOtherHand()->GetPartsPicker()->GetHidden();

	constexpr float minAngle = 50.0f;
	constexpr float maxAngle = 90.0f;

	if (_handIndex == 0)
	{
		_partsPicker->SetHidden(!otherPickerHidden || handRotation.z > minAngle || handRotation.z < -maxAngle);
	}
	else if (_handIndex == 1)
	{
		_partsPicker->SetHidden(!otherPickerHidden || handRotation.z < -minAngle || handRotation.z > maxAngle);
	}
}

void Hand::UpdateFingers(float /*delta*/)
{
	World *world = World::GetSharedInstance();
	RN::VRCamera *vrCamera = world->GetVRCamera();

	// get new state
	const RN::VRHandTrackingState hand = vrCamera->GetHandTrackingState(_handIndex);

	if (!hand.tracking) { return; }

	// update state
	_wasPinching = _pinching;
	_pinching = {false};

	// get joint states
	const auto &palm = hand.joints[Joint::Palm];
	const auto &thumb = hand.joints[Joint::ThumbTip];

	const auto &index = hand.joints[Joint::IndexTip];
	const auto &middle = hand.joints[Joint::MiddleTip];
	const auto &ring = hand.joints[Joint::RingTip];
	const auto &little = hand.joints[Joint::LittleTip];

	// hand.tracking doesnt seem to be accurate
	if (!_hasStartedTracking && palm.position.GetLength() > RN::k::EpsilonFloat)
	{
		_hasStartedTracking = true;
	}

	// update pinching state
	constexpr float pinchThreshold = 0.02f; // 2 cm

	const std::array<const RN::VRHandTrackingState::JointState *, Pinch::_Count> tips =
		{&index, &middle, &ring, &little};

	for (size_t i = 0; i < Pinch::_Count; i++)
	{
		const auto *tip = tips[i];
		const float distance = (tip->position - thumb.position).GetLength();
		const bool pinching = distance < pinchThreshold;
		_pinching[i] = pinching;
	}

	// update indicators
	for (size_t i = 0; i < Joint::_JointCount; i++)
	{
		const auto &joint = hand.joints[i];
		GetJointIndicator(i)->SetPosition(joint.position);
		GetJointIndicator(i)->SetRotation(joint.rotation);
	}
}

void Hand::TryGrabObject()
{
	World *world = World::GetSharedInstance();

	// get physics object
	auto *object = GetPinchOverlap<PhysicsGroup>(Types::CollisionGrabbable | Types::CollisionGrabbing);
	if (object)
	{
		GrabObject(object);
		return;
	}

	// get parts menu object
	auto *part = GetPinchOverlap<Part>(Types::CollisionPartPicker);
	if (part)
	{
		auto *object = _partsPicker->CreatePhysicsObjectForPart(part);

		object->SetWorldPosition(part->GetWorldPosition());
		world->AddLevelNode(object->Autorelease());

		GrabObject(object);
		return;
	}
}

void Hand::GrabObject(PhysicsGroup *object)
{
	World *world = World::GetSharedInstance();

	_grabbedObject = SafeRetain(object);

	const RN::Vector3 handPosition = GetPinchTarget();
	const RN::Quaternion handRotation = GetPinchRotation();

	const RN::Vector3 objectPosition = object->GetWorldPosition();
	const RN::Quaternion objectRotation = object->GetWorldRotation();

	_grabRotationOffset = handRotation.GetInverse() * objectRotation;
	_grabPositionOffset = handRotation.GetInverse().GetRotatedVector(objectPosition - handPosition);

	auto *otherHand = GetOtherHand();

	if (object == otherHand->GetGrabbedObject())
	{
		_scaling = true;

		const RN::Vector3 p0 = GetPinchTarget();
		const RN::Vector3 p1 = otherHand->GetPinchTarget();

		const RN::Vector3 objectPosition = object->GetWorldPosition();
		const RN::Quaternion objectRotation = object->GetWorldRotation();
		const RN::Quaternion invRotation = objectRotation.GetInverse();

		// object local anchors
		_initialGrabLocal0 = invRotation.GetRotatedVector(p0 - objectPosition);
		_initialGrabLocal1 = invRotation.GetRotatedVector(p1 - objectPosition);

		// base values
		_initialHandDistance = (p1 - p0).GetLength();
		_initialObjectScale = object->GetScale();

		// remove physics
		_grabbedObject->StartManipulating();
	}
	else
	{
		// disable physics
		_grabbedObject->Grab();
	}
}

void Hand::DropObject()
{
	if (!_grabbedObject) { return; }

	World *world = World::GetSharedInstance();
	auto *otherHand = GetOtherHand();
	auto *otherObject = otherHand->GetGrabbedObject();

	// combining cubes
	if (otherObject && otherObject != _grabbedObject)
	{
		auto *shape = _grabbedObject->GetPhysicsBody()->GetShape();
		const RN::Vector3 position = _grabbedObject->GetWorldPosition();
		const RN::Quaternion rotation = _grabbedObject->GetWorldRotation();

		auto *object = Physics::TestOverlap<PhysicsGroup>(
			shape, position, rotation, Types::CollisionGrabbing, _grabbedObject);

		if (object && object != _grabbedObject)
		{
			otherObject->Merge(_grabbedObject);

			SafeRelease(_grabbedObject);
			return;
		}
	}

	if (_scaling)
	{
		_scaling = false;

		// resetup physics
		_grabbedObject->StopManipulating();

		// update other hand offsets
		const RN::Vector3 handPosition = otherHand->GetPinchTarget();
		const RN::Quaternion handRotation = otherHand->GetPinchRotation();

		const RN::Vector3 objectPosition = _grabbedObject->GetWorldPosition();
		const RN::Quaternion objectRotation = _grabbedObject->GetWorldRotation();

		otherHand->_grabRotationOffset = handRotation.GetInverse() * objectRotation;
		otherHand->_grabPositionOffset = handRotation.GetInverse().GetRotatedVector(objectPosition - handPosition);
	}
	else
	{
		_grabbedObject->Throw(_linearVelocity, _angularVelocity);
	}

	SafeRelease(_grabbedObject);
}

template <class T>
T *Hand::GetPinchOverlap(RN::uint32 mask)
{
	World *world = World::GetSharedInstance();

	const RN::Vector3 pinch = GetPinchTarget();

	// check for overlaps at the pinch position
	auto object = Physics::TestOverlap<T>(
		_intersectShape, pinch, RN::Quaternion(), mask);

	return object;
}

bool Hand::IsMainHand() const
{
	constexpr RN::uint8 mainHandIndex = 0;
	return _handIndex == mainHandIndex;
}

Hand *Hand::GetOtherHand() const
{
	World *world = World::GetSharedInstance();
	auto *otherHand = world->GetHand(1 - _handIndex);
	return otherHand;
}

// pinch target is the point in between the index and thumb
RN::Vector3 Hand::GetPinchTarget() const
{
	auto *index = GetJointIndicator(Joint::IndexTip);
	auto *thumb = GetJointIndicator(Joint::ThumbTip);

	const RN::Vector3 indexPosition = index->GetPosition();
	const RN::Vector3 thumbPosition = thumb->GetPosition();

	const RN::Vector3 midpoint = (indexPosition + thumbPosition) * 0.5f;

	return midpoint;
}

// pinch rotation is just based off the index
RN::Quaternion Hand::GetPinchRotation() const
{
	auto *index = GetJointIndicator(Joint::IndexTip);
	return index->GetRotation();
}

bool Hand::IsPinching(size_t pinch) const
{
	return _pinching.at(pinch);
}

bool Hand::WasPinching(size_t pinch) const
{
	return _wasPinching.at(pinch);
}

RN::Entity *Hand::GetJointIndicator(size_t joint) const
{
	return _indicator.at(joint);
}

} // namespace CG
