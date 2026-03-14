#include "Hand.hpp"

#include "PartsPicker.hpp"
#include "PhysicsCube.hpp"
#include "Types.hpp"
#include "World.hpp"

namespace CG
{

Hand::Hand(uint8_t index)
	: _handIndex(index), _grabbedObject(nullptr), _scaling(false)
{
	World *world = World::GetSharedInstance();

	// small cubes to indicate hand location
	auto *model = world->AssignShader(RN::Model::WithCube(RN::Color::Blue()), Types::MaterialType::MaterialDefault);

	for (size_t i = 0; i < 6; i++)
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
	auto *palm = _indicator.at(0);
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
	World *world = World::GetSharedInstance();
	auto *index = _indicator.at(2);

	if (_pinching.at(0))
	{
		// pinch to grab
		if (!_grabbedObject && !_wasPinching.at(0))
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

	// accumulate velocity
	const RN::Vector3 linearVelocity = (index->GetWorldPosition() - _previousPosition) / delta;
	const RN::Vector3 angularVelocity = _previousRotation.GetAngularVelocity(index->GetWorldRotation(), delta);

	_linearVelocity = _linearVelocity * 0.1 + linearVelocity * 0.9;
	_angularVelocity = _angularVelocity * 0.1 + angularVelocity * 0.9;

	// next frame info
	_previousPosition = index->GetWorldPosition();
	_previousRotation = index->GetWorldRotation();
}

void Hand::UpdateMovingObject(float delta)
{
	auto *index = _indicator.at(2);

	const RN::Vector3 handPosition = index->GetWorldPosition();
	const RN::Quaternion handRotation = index->GetWorldRotation();

	const RN::Vector3 worldOffset = handRotation.GetRotatedVector(_grabPositionOffset);

	const RN::Vector3 position = handPosition + worldOffset;
	const RN::Quaternion rotation = (handRotation * _grabRotationOffset).Normalize();

	_grabbedObject->GetPhysicsBody()->SetKinematicTarget(position, rotation, delta);
}

void Hand::UpdateScalingObject(float /*delta*/)
{
	if (_handIndex == 1) { return; } // only do scaling logic once

	World *world = World::GetSharedInstance();

	auto *otherHand = world->GetHand(1);
	auto *thisIndex = _indicator.at(2);
	auto *otherIndex = otherHand->_indicator.at(2);

	const RN::Vector3 p0 = thisIndex->GetWorldPosition();
	const RN::Vector3 p1 = otherIndex->GetWorldPosition();

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
	auto *palm = _indicator.at(0);
	const auto handRotation = palm->GetWorldEulerAngle();

	bool otherPickerHidden = World::GetSharedInstance()->GetHand(1 - _handIndex)->GetPartsPicker()->GetHidden();

	if (_handIndex == 0)
	{
		_partsPicker->SetHidden(!otherPickerHidden || handRotation.z > 30 || handRotation.z < -90);
	}
	else if (_handIndex == 1)
	{
		_partsPicker->SetHidden(!otherPickerHidden || handRotation.z < -30 || handRotation.z > 90);
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
	const auto palm = hand.joints[RN::VRHandTrackingState::Joint::Palm];
	const auto thumb = hand.joints[RN::VRHandTrackingState::Joint::ThumbTip];
	const auto index = hand.joints[RN::VRHandTrackingState::Joint::IndexTip];
	const auto middle = hand.joints[RN::VRHandTrackingState::Joint::MiddleTip];
	const auto ring = hand.joints[RN::VRHandTrackingState::Joint::RingTip];
	const auto little = hand.joints[RN::VRHandTrackingState::Joint::LittleTip];

	// get pinch distances
	const float indexDistance = (index.position - thumb.position).GetLength();
	const float middleDistance = (middle.position - thumb.position).GetLength();
	const float ringDistance = (ring.position - thumb.position).GetLength();
	const float littleDistance = (little.position - thumb.position).GetLength();

	// update pinching state
	const float pinchThreshold = 0.02f; // 1 cm

	const bool indexPinching = indexDistance < pinchThreshold;
	const bool middlePinching = middleDistance < pinchThreshold;
	const bool ringPinching = ringDistance < pinchThreshold;
	const bool littlePinching = littleDistance < pinchThreshold;

	_pinching = {indexPinching, ringPinching, middlePinching, littlePinching};

	// update indicators
	_indicator.at(0)->SetPosition(palm.position);
	_indicator.at(0)->SetRotation(palm.rotation);
	_indicator.at(1)->SetPosition(thumb.position);
	_indicator.at(1)->SetRotation(thumb.rotation);
	_indicator.at(2)->SetPosition(index.position);
	_indicator.at(2)->SetRotation(index.rotation);
	_indicator.at(3)->SetPosition(middle.position);
	_indicator.at(3)->SetRotation(middle.rotation);
	_indicator.at(4)->SetPosition(ring.position);
	_indicator.at(4)->SetRotation(ring.rotation);
	_indicator.at(5)->SetPosition(little.position);
	_indicator.at(5)->SetRotation(little.rotation);
}

void Hand::TryGrabObject()
{
	World *world = World::GetSharedInstance();
	auto *index = _indicator.at(2);

	// get colliding cube
	auto overlaps = world->GetPhysicsWorld()->CheckOverlap(_intersectShape, index->GetWorldPosition(), RN::Quaternion(), 1.0f, Types::CollisionTest, Types::CollisionGrabbable);
	for (const auto &info : overlaps)
	{
		if (!info.node) { continue; }
		auto *cube = info.node->Downcast<PhysicsCube>();
		if (!cube) { continue; }

		GrabObject(cube);
		return;
	}

	// get parts menu cube
	overlaps = world->GetPhysicsWorld()->CheckOverlap(_intersectShape, index->GetWorldPosition(), RN::Quaternion(), 1.0f, Types::CollisionTest, Types::CollisionPartPicker);
	for (const auto &info : overlaps)
	{
		if (!info.node) { continue; }
		auto *cube = info.node->Downcast<RN::Entity>();
		if (!cube) { continue; }

		auto *physicsCube = new PhysicsCube(cube->GetModel());
		physicsCube->SetPosition(cube->GetWorldPosition());
		world->AddLevelNode(physicsCube->Autorelease());

		GrabObject(physicsCube);
		return;
	}
}

void Hand::GrabObject(PhysicsCube *object)
{
	World *world = World::GetSharedInstance();

	auto *index = _indicator.at(2);
	_grabbedObject = SafeRetain(object);

	const RN::Vector3 handPosition = index->GetWorldPosition();
	const RN::Quaternion handRotation = index->GetWorldRotation();

	const RN::Vector3 objectPosition = object->GetWorldPosition();
	const RN::Quaternion objectRotation = object->GetWorldRotation();

	_grabRotationOffset = handRotation.GetInverse() * objectRotation;
	_grabPositionOffset = handRotation.GetInverse().GetRotatedVector(objectPosition - handPosition);

	auto *otherHand = world->GetHand(1 - _handIndex);

	if (object == otherHand->GetGrabbedObject())
	{
		_scaling = true;
		otherHand->_scaling = true;

		auto *thisIndex = _indicator.at(2);
		auto *otherIndex = otherHand->_indicator.at(2);
		auto *leftHand = world->GetHand(0); // store values in left hand

		const RN::Vector3 p0 = thisIndex->GetWorldPosition();
		const RN::Vector3 p1 = otherIndex->GetWorldPosition();

		const RN::Vector3 objectPosition = object->GetWorldPosition();
		const RN::Quaternion objectRotation = object->GetWorldRotation();
		const RN::Quaternion invRotation = objectRotation.GetInverse();

		// object local anchors
		leftHand->_initialGrabLocal0 = invRotation.GetRotatedVector(p0 - objectPosition);
		leftHand->_initialGrabLocal1 = invRotation.GetRotatedVector(p1 - objectPosition);

		// base values
		leftHand->_initialHandDistance = (p1 - p0).GetLength();
		leftHand->_initialObjectScale = object->GetScale();

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

	if (_scaling)
	{
		auto *otherHand = world->GetHand(1 - _handIndex);
		auto *index = otherHand->_indicator.at(2);

		_scaling = false;
		otherHand->_scaling = false;

		// resetup physics
		_grabbedObject->StopManipulating();

		// update other hand offsets
		const RN::Vector3 handPosition = index->GetWorldPosition();
		const RN::Quaternion handRotation = index->GetWorldRotation();

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

} // namespace CG
