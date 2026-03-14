#include <Rayne.h>

#include "CubeGameLib.hpp"

#if RN_BUILD_RELEASE
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#if RN_PLATFORM_VISIONOS
void visionos_main(cp_layer_renderer_t layerRenderer)
{
	RN::Initialize(0, nullptr, new CG::Application(), layerRenderer);
}
#elif RN_PLATFORM_IOS
void ios_main(CAMetalLayer *metalLayer)
{
	RN::Initialize(0, nullptr, new CG::Application(), metalLayer);
}
#elif RN_PLATFORM_ANDROID
void android_main(struct android_app *app)
{
	RN::Initialize(0, nullptr, new CG::Application(), app);
}
#else
int main(int argc, const char *argv[])
{
#if RN_BUILD_DEBUG
//	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
#endif

	RN::Initialize(argc, argv, new CG::Application());
}
#endif
