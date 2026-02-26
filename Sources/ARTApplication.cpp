//
//  ARTApplication.cpp
//  AR-Test
//
//  Copyright 2026 by index. All rights reserved.
//

#include "ARTApplication.h"
#include "ARTWorld.h"
#include "RNVRApplicationImpl.h"

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
		RN::Application::WillFinishLaunching(kernel);

		if(!RN::Kernel::GetSharedInstance()->GetArguments().HasArgument("pancake", '2d'))
		{
			SetupVR();
		}
	}

	void Application::DidFinishLaunching(RN::Kernel *kernel)
	{
		RN::VRApplication::DidFinishLaunching(kernel);
		
#if RN_PLATFORM_ANDROID
		RN::Shader::ArgumentSampler::SetDefaultAnisotropy(4);
#else
		RN::Shader::ArgumentSampler::SetDefaultAnisotropy(16);
#endif

		World *world = new World(GetVRWindow());
		RN::SceneManager::GetSharedInstance()->AddScene(world->Autorelease());
	}
}
