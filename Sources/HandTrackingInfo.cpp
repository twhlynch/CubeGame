#include "HandTrackingInfo.hpp"

#include "World.hpp"

namespace CG
{

static constexpr float resolution = 1000.0f;
static constexpr float fontSize = 120.0f;

HandTrackingInfo::HandTrackingInfo()
	: _window(new RN::UI::Window(RN::Rect(0.0f, 0.0f, resolution, fontSize * 2)))
{
	_window->SetRenderGroup(1); // UI defaults to 1 << 7 not visible by the vr camera
	_window->SetBackgroundColor(RN::Color::ClearColor());
	_window->SetScale(1.0f / resolution);
	_window->SetPosition(RN::Vector3(-0.5f, 0.0f, 0.0f));

	auto *font = RN::UI::FontManager::GetSharedInstance()->GetFontForFilepath(RNCSTR("fonts/Roboto-Bold.ttf"));
	auto *label = new RN::UI::Label(RN::UI::TextAttributes(font, fontSize, RN::Color::White(), RN::UI::TextAlignmentCenter));
	label->SetVerticalAlignment(RN::UI::TextVerticalAlignmentCenter);
	label->SetText(RNCSTR("Swap to hand tracking to play"));
	label->SetFrame(_window->GetBounds());
	_window->AddSubview(label->Autorelease());

	AddChild(_window->Autorelease());
}

void HandTrackingInfo::Update(float delta)
{
	World *world = World::GetSharedInstance();

	auto *leftHand = world->GetHand(0);
	auto *rightHand = world->GetHand(1);

	if (leftHand->GetHasStartedTracking() && rightHand->GetHasStartedTracking())
	{
		world->RemoveLevelNode(this);
		return;
	}

	auto *camera = world->GetHeadCamera();
	if (!camera) { return; }

	// smoothly float in front of camera
	const RN::Vector3 position = camera->GetWorldPosition();
	const RN::Vector3 forward = camera->GetForward();

	const RN::Vector3 targetPosition = position + (forward * 0.5f);
	const RN::Vector3 currentPosition = GetWorldPosition();

	const float factor = std::min(30.0f * delta, 1.0f);
	const RN::Vector3 resultPosition = currentPosition.GetLerp(targetPosition, factor);

	SetWorldPosition(resultPosition);

	// look at camera
	SetWorldRotation(RN::Quaternion::WithLookAt(-forward));
}

}; // namespace CG
