#include "ARTHand.h"

#include <RNVRCamera.h>

#include "ARTWorld.h"

namespace ART
{

Hand::Hand(uint8_t index)
	: _handIndex(index), _pinching(false), _wasPinching(false)
{
	World *world = World::GetSharedInstance();

	// small cube to indicate hand location
	RN::Model *cubeModel = world->AssignShader(RN::Model::WithCube(RN::Color::Blue()), Types::MaterialType::MaterialDefault);
	auto *cubeEntity = new RN::Entity(cubeModel);
	cubeEntity->SetScale(0.01f);
	AddChild(cubeEntity->Autorelease());

	auto *cubePhysicsMaterial = new RN::JoltMaterial();
	auto *cubeShape = RN::JoltCompoundShape::WithModel(cubeModel, cubePhysicsMaterial->Autorelease(), RN::Vector3(1.0f, 1.0f, 1.0f), true);
	auto *cubeBody = RN::JoltStaticBody::WithShape(cubeShape);
	cubeBody->SetCollisionFilter(Types::CollisionLevel, Types::CollisionAll);
	cubeEntity->AddAttachment(cubeBody);
}

void Hand::Update(float /*delta*/)
{
	RN::VRCamera *vrCamera = World::GetSharedInstance()->GetVRCamera();
	if (!vrCamera) { return; }

	// update state
	_wasPinching = _pinching;
	_pinching = false;

	const RN::VRHandTrackingState hand = vrCamera->GetHandTrackingState(_handIndex);

	if (hand.tracking)
	{
		// update position rotation
		SetWorldPosition(vrCamera->GetWorldPosition() + hand.position);
		SetWorldRotation(hand.rotation);

		// update pinching state
		const float pinchThreshold = 0.01f; // 1 cm

		const bool indexPinching = hand.pinchDistance[RN::VRHandTrackingState::PinchFingerIndex] < pinchThreshold;
		const bool ringPinching = hand.pinchDistance[RN::VRHandTrackingState::PinchFingerRing] < pinchThreshold;
		const bool middlePinching = hand.pinchDistance[RN::VRHandTrackingState::PinchFingerMiddle] < pinchThreshold;
		const bool pinkyPinching = hand.pinchDistance[RN::VRHandTrackingState::PinchFingerPinky] < pinchThreshold;

		_pinching = indexPinching || ringPinching || middlePinching || pinkyPinching;
	}
}

} // namespace ART
