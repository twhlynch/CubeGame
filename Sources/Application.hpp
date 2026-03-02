#pragma once

#include <RNVRApplication.h>
#include <Rayne.h>

namespace ART
{
class Application : public RN::VRApplication
{
public:
	Application();
	~Application() override;

	void WillFinishLaunching(RN::Kernel *kernel) override;
	void DidFinishLaunching(RN::Kernel *kernel) override;
};
} // namespace ART
