#include "ARTApplication.h"

#include <RNVRApplicationImpl.h>

#include "ARTWorld.h"

namespace ART
{
Application::Application()
{
}

Application::~Application()
{
}

void Application::WillFinishLaunching(RN::Kernel *kernel)
{
	RN::Application::WillFinishLaunching(kernel); // NOLINT(bugprone-parent-virtual-call)

	if (!RN::Kernel::GetSharedInstance()->GetArguments().HasArgument("pancake", '2d')) // NOLINT
	{
		SetupVR();
	}
}

void Application::DidFinishLaunching(RN::Kernel *kernel)
{
	RN::VRApplication::DidFinishLaunching(kernel);

	if (!GetVRWindow() || !GetVRWindow()->EnableHandTracking())
	{
		RNDebug("EnableHandTracking failed");
	}

#if RN_PLATFORM_ANDROID
	RN::Shader::ArgumentSampler::SetDefaultAnisotropy(4);
#else
	RN::Shader::ArgumentSampler::SetDefaultAnisotropy(16);
#endif

	auto *world = new World(GetVRWindow());
	RN::SceneManager::GetSharedInstance()->AddScene(world->Autorelease());
}
} // namespace ART
