#include "Menu.hpp"

#include <Debug/RNLogger.h>
#include <Math/RNVector.h>
#include <Scene/RNEntity.h>

#include "World.hpp"

namespace CG
{

RNDefineMeta(Menu, RN::SceneNode);

static constexpr float resolution = 1000.0f;
static constexpr float fontSize = 30.0f;

Menu::Menu()
	: _window(new RN::UI::Window(RN::Rect(0.0f, 0.0f, resolution, fontSize * 2))),
	  _hidden(true)
{
	_window->SetRenderGroup(1); // UI defaults to 1 << 7 not visible by the vr camera
	_window->SetBackgroundColor(RN::Color::ClearColor());
	_window->SetScale(1.0f / resolution);
	_window->SetPosition(RN::Vector3(-0.5f, 0.0f, 0.0f));

	auto *font = RN::UI::FontManager::GetSharedInstance()->GetFontForFilepath(RNCSTR("fonts/Roboto-Bold.ttf"));

	constexpr float buttonHeight = fontSize * 1.2f;
	constexpr float buttonWidth = buttonHeight * 5.0f;

	_resetButton = new RN::UI::Button(RN::UI::TextAttributes(font, fontSize, RN::Color::White(), RN::UI::TextAlignmentCenter));
	_resetButton->GetLabel()->SetVerticalAlignment(RN::UI::TextVerticalAlignmentCenter);
	_resetButton->GetLabel()->SetText(RNCSTR("Reset Scene"));
	_resetButton->SetBackgroundColor(RN::Color(1.0f, 0.0f, 0.0f, 0.8f));
	_resetButton->SetCornerRadius(fontSize);
	// centered
	_resetButton->SetFrame(
		{
			(resolution * 0.5f) - (buttonWidth * 0.5f),
			fontSize - (buttonHeight * 0.5f),
			buttonWidth,
			buttonHeight,
		});

	_window->AddSubview(_resetButton->Autorelease());

	_window->SetHidden(_hidden);
	AddChild(_window->Autorelease());
}

void Menu::Toggle()
{
	_hidden = !_hidden;

	_window->SetHidden(_hidden);
}

void Menu::Update(float delta)
{
	World *world = World::GetSharedInstance();
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

	// handle clicking
	if (_hidden) { return; }

	_resetButton->SetIsHighlighted(false);

	for (size_t i = 0; i < 2; i++)
	{
		auto *hand = world->GetHand(i);

		const RN::Vector3 indexPosition = hand->GetFingerTipPosition(Joint::IndexTip);

		const RN::Vector3 windowPosition = _window->GetWorldPosition();
		const RN::Vector3 normal = _window->GetForward();
		const RN::Vector3 right = _window->GetRight();
		const RN::Vector3 up = _window->GetUp();

		const RN::Vector3 toPoint = indexPosition - windowPosition;
		const float distance = toPoint.GetDotProduct(normal);

		// project
		const RN::Vector3 projected = indexPosition - normal * distance;
		const RN::Vector3 local = projected - windowPosition;

		// convert to 2d
		const float x = local.GetDotProduct(right) * resolution;
		const float y = -local.GetDotProduct(up) * resolution;

		const bool isTouchingMenu = (distance < 0.02f && distance > -0.02f);
		if (isTouchingMenu)
		{
			_window->UpdateCursorPosition({x, y});
			HandleButtonClick();
		}
	}
}

void Menu::HandleButtonClick()
{
	if (_resetButton->GetIsHighlighted())
	{
		World::GetSharedInstance()->LoadLevel();
		Toggle();
	}
}

} // namespace CG
