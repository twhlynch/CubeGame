#pragma once

#include <RNVRCamera.h>
#include <Rayne.h>

namespace ART
{
class CameraManager
{
public:
	CameraManager();
	~CameraManager();

	void Setup(RN::VRWindow *vrWindow);

	RN::VRHMDTrackingState::Mode Update(float delta);
	void SetPreviewWindowEnabled(bool enable);
	void SetPreviewCameraEnabled(bool enable);
	void ResetPositionAndRotationDelayed();

	void SetCameraAmbientColor(const RN::Color &targetColor, float changeRate, std::function<void(void)> completed);
	[[nodiscard]] RN::SceneNode *GetHeadSceneNode() const;

	[[nodiscard]] RN::VRCamera *GetVRCamera() const { return _vrCamera; }
	[[nodiscard]] RN::Camera *GetHeadCamera() const { return _headCamera; }
	[[nodiscard]] RN::Camera *GetPreviewCamera() const { return _previewCamera; }

protected:
	void UpdateForWindowSize() const;
	void UpdateCameraAmbientColor(float delta);
	void MovePancakeCamera(float delta);
	void UpdatePreviewCamera(float delta);
	void ResetPositionAndRotation();

	void ClearPipeline();
	void GeneratePipeline();
	void RegeneratePipeline();

	RN::VRWindow *_vrWindow;
	RN::VRCamera *_vrCamera;
	RN::Camera *_headCamera;

	RN::Camera *_previewCamera;
	RN::Window *_previewWindow;
	RN::Material *_copyEyeToScreenMaterial;

	RN::Window *_vrDebugWindow;

	RN::Color _cameraTargetAmbientColor;
	RN::Color _cameraTargetAmbientColorChangeRate;
	std::function<void(void)> _cameraTargetAmbientColorCompletedCallback;
	bool _cameraTargetAmbientColorIsWaitingForLastFrame;

	RN::Vector2 _defaultPreviewWindowResolution;
	bool _wantsPreviewWindow;
	bool _wantsPreviewCamera;
	bool _wantsVRDebugWindow;
	RN::uint8 _msaa;

	bool _resetPositionAndRotation;

	RN::VRCompositorLayer *_passthroughLayer;
};
} // namespace ART
