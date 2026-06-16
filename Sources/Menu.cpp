#include "Menu.hpp"

#include <Debug/RNLogger.h>
#include <Math/RNVector.h>
#include <Scene/RNEntity.h>

#include "PhysicsGroup.hpp"
#include "StructureHelpers.hpp"
#include "World.hpp"

namespace CG
{

RNDefineMeta(Menu, RN::SceneNode);

static constexpr float resolution = 1000.0f;
static constexpr float fontSize = 30.0f;

Menu::Menu()
	: _window(new RN::UI::Window(RN::Rect(0.0f, 0.0f, resolution, fontSize * 14))),
	  _hidden(true)
{
	_window->SetRenderGroup(1); // UI defaults to 1 << 7 not visible by the vr camera
	_window->SetBackgroundColor(RN::Color::ClearColor());
	_window->SetScale(1.0f / resolution);
	_window->SetPosition(RN::Vector3(-0.5f, 0.0f, 0.0f));

	auto *font = RN::UI::FontManager::GetSharedInstance()->GetFontForFilepath(RNCSTR("fonts/Roboto-Bold.ttf"));

	constexpr float buttonHeight = fontSize * 1.2f;
	constexpr float buttonWidth = buttonHeight * 5.0f;

	const RN::Rect centered = {
		(resolution * 0.5f) - (buttonWidth * 0.5f),
		(fontSize * 0.6f) + fontSize - (buttonHeight * 0.5f),
		buttonWidth,
		buttonHeight,
	};

	// version
	auto *versionLabel = new RN::UI::Label(RN::UI::TextAttributes(font, fontSize * 0.4f, RN::Color::White(), RN::UI::TextAlignmentCenter));
	versionLabel->SetVerticalAlignment(RN::UI::TextVerticalAlignmentCenter);
	versionLabel->SetText(RNCSTR("Cube Game v" CG_VERSION_STRING " (Build " CG_BUILD_NUMBER ")"));
	versionLabel->SetFrame(RN::Rect(0.0f, 0.0f, resolution, fontSize * 0.6f));
	_window->AddSubview(versionLabel->Autorelease());

	// buttons
	_resetButton = new RN::UI::Button(RN::UI::TextAttributes(font, fontSize, RN::Color::White(), RN::UI::TextAlignmentCenter));
	_resetButton->GetLabel()->SetVerticalAlignment(RN::UI::TextVerticalAlignmentCenter);
	_resetButton->GetLabel()->SetText(RNCSTR("Reset Scene"));
	_resetButton->SetBackgroundColor(RN::Color(1.0f, 0.0f, 0.0f, 0.8f));
	_resetButton->SetCornerRadius(fontSize);
	_resetButton->SetFrame(centered + RN::Rect(buttonWidth * -0.6f, 0, 0, 0));

	_debugButton = new RN::UI::Button(RN::UI::TextAttributes(font, fontSize * 0.8f, RN::Color::White(), RN::UI::TextAlignmentCenter));
	_debugButton->GetLabel()->SetVerticalAlignment(RN::UI::TextVerticalAlignmentCenter);
	_debugButton->GetLabel()->SetText(RNCSTR("Toggle Debug"));
	_debugButton->SetBackgroundColor(RN::Color(0.0f, 0.8f, 0.0f, 0.8f));
	_debugButton->SetCornerRadius(fontSize);
	_debugButton->SetFrame(centered + RN::Rect(buttonWidth * 0.6f, 0, 0, 0));

	_wallButton = new RN::UI::Button(RN::UI::TextAttributes(font, fontSize, RN::Color::White(), RN::UI::TextAlignmentCenter));
	_wallButton->GetLabel()->SetVerticalAlignment(RN::UI::TextVerticalAlignmentCenter);
	_wallButton->GetLabel()->SetText(RNCSTR("Add Wall"));
	_wallButton->SetBackgroundColor(RN::Color(0.0f, 0.0f, 0.8f, 0.8f));
	_wallButton->SetCornerRadius(fontSize);
	_wallButton->SetFrame(centered + RN::Rect(buttonWidth * -0.6f, buttonHeight * 2, 0, 0));

	_towersButton = new RN::UI::Button(RN::UI::TextAttributes(font, fontSize, RN::Color::White(), RN::UI::TextAlignmentCenter));
	_towersButton->GetLabel()->SetVerticalAlignment(RN::UI::TextVerticalAlignmentCenter);
	_towersButton->GetLabel()->SetText(RNCSTR("Add Towers"));
	_towersButton->SetBackgroundColor(RN::Color(0.0f, 0.0f, 0.8f, 0.8f));
	_towersButton->SetCornerRadius(fontSize);
	_towersButton->SetFrame(centered + RN::Rect(buttonWidth * 0.6f, buttonHeight * 2, 0, 0));

	// lan server
	_lanButton = new RN::UI::Button(RN::UI::TextAttributes(font, fontSize, RN::Color::White(), RN::UI::TextAlignmentCenter));
	_lanButton->GetLabel()->SetVerticalAlignment(RN::UI::TextVerticalAlignmentCenter);
	_lanButton->GetLabel()->SetText(RNCSTR("Open to LAN"));
	_lanButton->SetBackgroundColor(RN::Color(0.5f, 0.0f, 0.5f, 0.8f));
	_lanButton->SetCornerRadius(fontSize);
	_lanButton->SetFrame(centered + RN::Rect(0, buttonHeight * 6, 0, 0));

	_lanLabel = new RN::UI::Label(RN::UI::TextAttributes(font, fontSize * 0.5f, RN::Color(0.5f, 0.5f, 0.5f, 0.6f), RN::UI::TextAlignmentCenter));
	_lanLabel->SetVerticalAlignment(RN::UI::TextVerticalAlignmentCenter);
	_lanLabel->SetText(RNCSTR(""));
	_lanLabel->SetFrame(RN::Rect(0, (buttonHeight * 5.5) + fontSize, resolution, fontSize * 0.6f));

	_window->AddSubview(_resetButton->Autorelease());
	_window->AddSubview(_wallButton->Autorelease());
	_window->AddSubview(_towersButton->Autorelease());
	_window->AddSubview(_debugButton->Autorelease());
	_window->AddSubview(_lanButton->Autorelease());
	_window->AddSubview(_lanLabel->Autorelease());

	// sliders
	constexpr float sliderWidth = 190.0f;
	constexpr float sliderHeight = fontSize * 1.2f;
	constexpr float sliderY = (buttonHeight * 4.0f) + sliderHeight;
	constexpr float labelY = sliderY - (fontSize * 0.5f);

	const float sliderGap = (resolution - sliderWidth * 2.0f) / 3.0f;
	const float gravitySliderX = sliderGap + (sliderWidth * 0.5f);
	const float massSliderX = (sliderGap * 2.0f) + sliderWidth - (sliderWidth * 0.5f);

	const RN::UI::TextAttributes sliderLabelAttr(font, fontSize * 0.55f, RN::Color::White(), RN::UI::TextAlignmentLeft);

	_gravityLabel = new RN::UI::Label(sliderLabelAttr);
	_gravityLabel->SetText(RNCSTR("Gravity: 9.81"));
	_gravityLabel->SetFrame(RN::Rect(gravitySliderX, labelY, sliderWidth, fontSize * 0.6f));

	_massLabel = new RN::UI::Label(sliderLabelAttr);
	_massLabel->SetText(RNCSTR("Mass: 0.50"));
	_massLabel->SetFrame(RN::Rect(massSliderX, labelY, sliderWidth, fontSize * 0.6f));

	_gravitySlider = new RN::UI::Slider(RN::Rect(gravitySliderX, sliderY, sliderWidth, sliderHeight), 9.81f, 0.0f, 20.0f, 0.0f);
	_gravitySlider->GetRangeView()->SetBackgroundColor(RN::Color(0.6f, 0.6f, 0.6f, 1.0f));
	_gravitySlider->GetHandleView()->SetBackgroundColor(RN::Color(1.0f, 0.5f, 0.0f, 1.0f));
	_gravitySlider->GetHandleView()->SetCornerRadius(sliderHeight / 2.0f);

	_massSlider = new RN::UI::Slider(RN::Rect(massSliderX, sliderY, sliderWidth, sliderHeight), 0.5f, 0.1f, 5.0f, 0.0f);
	_massSlider->GetRangeView()->SetBackgroundColor(RN::Color(0.6f, 0.6f, 0.6f, 1.0f));
	_massSlider->GetHandleView()->SetBackgroundColor(RN::Color(0.0f, 0.8f, 1.0f, 1.0f));
	_massSlider->GetHandleView()->SetCornerRadius(sliderHeight / 2.0f);

	_window->AddSubview(_gravityLabel->Autorelease());
	_window->AddSubview(_massLabel->Autorelease());
	_window->AddSubview(_gravitySlider->Autorelease());
	_window->AddSubview(_massSlider->Autorelease());

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
	RN::SceneNode::Update(delta);

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

	// FIXME: recreating menu removes buttons background color, related to how its created
	_resetButton->SetBackgroundColor(RN::Color(1.0f, 0.0f, 0.0f, 0.8f));
	_wallButton->SetBackgroundColor(RN::Color(0.0f, 0.0f, 0.8f, 0.8f));
	_towersButton->SetBackgroundColor(RN::Color(0.0f, 0.0f, 0.8f, 0.8f));
	_debugButton->SetBackgroundColor(RN::Color(0.0f, 0.8f, 0.0f, 0.8f));
	_lanButton->SetBackgroundColor(RN::Color(0.5f, 0.0f, 0.5f, 0.8f));

	_resetButton->SetIsHighlighted(false);

	RN::Vector2 cursorPos;
	bool isTouching = false;

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

			cursorPos = {x, y};
			isTouching = true;
		}
	}

	// update sliders
	_gravitySlider->Update(delta, cursorPos, isTouching);
	_massSlider->Update(delta, cursorPos, isTouching);

	_gravityLabel->SetText(RN::String::WithFormat("Gravity: %.2f", _gravitySlider->GetValue()));
	_massLabel->SetText(RN::String::WithFormat("Mass: %.2f", _massSlider->GetValue()));

	world->GetPhysicsWorld()->SetGravity(RN::Vector3(0.0f, -_gravitySlider->GetValue(), 0.0f));
	PhysicsGroup::SetDefaultMass(_massSlider->GetValue());

	// update lan label
	auto *lanServer = world->GetLANServer();
	auto *httpServer = world->GetHTTPServer();
	if (lanServer->IsRunning())
	{
		const std::string &addr = lanServer->GetAddressString();
		uint16_t port = httpServer ? httpServer->GetPort() : 80;
		_lanLabel->SetText(RN::String::WithFormat("%s:%d", addr.c_str(), port));
	}
	else
	{
		_lanLabel->SetText(RNCSTR(""));
	}
}

void Menu::HandleButtonClick()
{
	if (_resetButton->GetIsHighlighted())
	{
		World::GetSharedInstance()->LoadLevel();
		Toggle();
	}
	else if (
		_wallButton->GetIsHighlighted() ||
		RN::InputManager::GetSharedInstance()->IsControlToggling(RNCSTR("1"))
	)
	{
		Structures::AddWall();
		Toggle();
	}
	else if (
		_towersButton->GetIsHighlighted() ||
		RN::InputManager::GetSharedInstance()->IsControlToggling(RNCSTR("2"))
	)
	{
		Structures::AddTowers();
		Toggle();
	}
	else if (_debugButton->GetIsHighlighted())
	{
		World::GetSharedInstance()->ToggleDebugMode();
		Toggle();
	}
	else if (_lanButton->GetIsHighlighted())
	{
		World *world = World::GetSharedInstance();
		auto *lanServer = world->GetLANServer();

		if (lanServer && lanServer->IsRunning())
		{
			world->StopLANServer();
			_lanButton->GetLabel()->SetText(RNCSTR("Open to LAN"));
			_lanLabel->SetText(RNCSTR(""));
		}
		else
		{
			world->StartLANServer();
			_lanButton->GetLabel()->SetText(RNCSTR("Close LAN"));
		}
		Toggle();
	}
}

} // namespace CG
