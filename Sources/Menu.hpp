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

	bool _hidden;

	RNDeclareMeta(Menu);
};

} // namespace CG
