#include "ARTHand.h"

#include "ARTPhysicsCube.h"
#include "ARTTypes.h"
#include "ARTWorld.h"

namespace ART
{

Hand::Hand(uint8_t index)
	: _handIndex(index), _grabbedObject(nullptr), _intersectShape(nullptr)
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

	if (_handIndex == 0) { UpdateLeftHand(delta); }
	else if (_handIndex == 1) { UpdateRightHand(delta); }
}

void Hand::UpdateLeftHand(float /*delta*/)
{
	World *world = World::GetSharedInstance();

	// pinch index to spawn cube
	if (_pinching.at(0) && !_wasPinching.at(0))
	{
		auto *index = _indicator.at(2);
		world->AddSmallCube(index->GetWorldPosition());
	}
}

void Hand::UpdateRightHand(float delta)
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

		// carrying
		if (_grabbedObject)
		{
			const RN::Vector3 handPosition = index->GetWorldPosition();
			const RN::Quaternion handRotation = index->GetWorldRotation();

			const RN::Vector3 worldOffset = handRotation.GetRotatedVector(_grabPositionOffset);

			const RN::Vector3 position = handPosition + worldOffset;
			const RN::Quaternion rotation = (handRotation * _grabRotationOffset).Normalize();

			_grabbedObject->GetPhysicsBody()->SetKinematicTarget(position, rotation, delta);
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
	auto overlaps = world->GetPhysicsWorld()->CheckOverlap(_intersectShape, index->GetWorldPosition(), RN::Quaternion(), 1.0f, Types::CollisionAll, Types::CollisionGrabbable);
	for (const auto &info : overlaps)
	{
		if (!info.node) { continue; }
		auto *cube = info.node->Downcast<PhysicsCube>();
		if (!cube) { continue; }

		GrabObject(cube);
		break;
	}
}

void Hand::GrabObject(PhysicsCube *object)
{
	auto *index = _indicator.at(2);
	_grabbedObject = SafeRetain(object);

	const RN::Vector3 handPosition = index->GetWorldPosition();
	const RN::Quaternion handRotation = index->GetWorldRotation();

	const RN::Vector3 objectPosition = object->GetWorldPosition();
	const RN::Quaternion objectRotation = object->GetWorldRotation();

	_grabRotationOffset = handRotation.GetInverse() * objectRotation;
	_grabPositionOffset = handRotation.GetInverse().GetRotatedVector(objectPosition - handPosition);

	auto *physicsBody = _grabbedObject->GetPhysicsBody();
	physicsBody->SetEnableKinematic(true);
	physicsBody->SetEnableGravity(false);
}

void Hand::DropObject()
{
	if (!_grabbedObject) { return; }

	auto *physicsBody = _grabbedObject->GetPhysicsBody();

	physicsBody->SetEnableKinematic(false);
	physicsBody->SetEnableGravity(true);

	physicsBody->SetLinearVelocity(_linearVelocity);
	physicsBody->SetAngularVelocity(_angularVelocity);

	SafeRelease(_grabbedObject);
}

} // namespace ART
