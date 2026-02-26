#pragma once

#if RN_PLATFORM_IOS || RN_PLATFORM_VISIONOS
#import <Foundation/Foundation.h>
#endif

#include "ARTApplication.h"
#include "ARTCameraManager.h"
#include "ARTTypes.h"
#include "ARTWorld.h"

#if RN_PLATFORM_VISIONOS
#import <CompositorServices/CompositorServices.h>
void visionos_main(cp_layer_renderer_t layerRenderer);
#elif RN_PLATFORM_IOS
#import <UIKit/UIKit.h>
void ios_main(CAMetalLayer *view);
#endif
