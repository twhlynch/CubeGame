//
//  ARTApplication.h
//  AR-Test
//
//  Copyright 2026 by index. All rights reserved.
//

#ifndef __ART_APPLICATION_H_
#define __ART_APPLICATION_H_

#include <Rayne.h>

#include "RNVRApplication.h"

namespace ART
{
class Application : public RN::VRApplication
{
public:
	Application();
	~Application();

	void WillFinishLaunching(RN::Kernel *kernel) override;
	void DidFinishLaunching(RN::Kernel *kernel) override;
};
} // namespace ART

#endif /* __ART_APPLICATION_H_ */
