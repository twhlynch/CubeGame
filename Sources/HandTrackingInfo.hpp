#pragma once

#include <RNUI.h>
#include <Rayne.h>

namespace CG
{

class HandTrackingInfo : public RN::SceneNode
{
public:
	HandTrackingInfo();

	void Update(float delta) override;

private:
	RN::UI::Window *_window;
};

} // namespace CG
