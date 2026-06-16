#pragma once

#include <RNUI.h>
#include <Rayne.h>

namespace CG
{

class Menu : public RN::SceneNode
{
public:
	Menu();

	void Update(float delta) override;
	void Toggle();

private:
	void HandleButtonClick();

	RN::UI::Window *_window;

	RN::UI::Button *_resetButton;
	RN::UI::Button *_debugButton;

	RN::UI::Button *_wallButton;
	RN::UI::Button *_towersButton;

	RN::UI::Slider *_gravitySlider;
	RN::UI::Label *_gravityLabel;

	RN::UI::Slider *_massSlider;
	RN::UI::Label *_massLabel;

	RN::UI::Button *_lanButton;
	RN::UI::Label *_lanLabel;

	bool _hidden;

	RNDeclareMeta(Menu);
};

} // namespace CG
