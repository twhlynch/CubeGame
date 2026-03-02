#pragma once

#include <RNJoltWorld.h>
#include <Rayne.h>

#include "CameraManager.hpp"
#include "Hand.hpp"
#include "Types.hpp"

namespace ART
{
class World : public RN::SceneBasic
{
public:
	static World *GetSharedInstance();
	static void Exit();

	World(RN::VRWindow *vrWindow);
	~World() override;

	RN::JoltWorld *GetPhysicsWorld() const { return _physicsWorld; }
	RN::ShaderLibrary *GetShaderLibrary() const { return _shaderLibrary; }
	RN::VRCamera *GetVRCamera() const { return _cameraManager.GetVRCamera(); }
	RN::Camera *GetHeadCamera() const { return _cameraManager.GetHeadCamera(); }
	RN::Camera *GetPreviewCamera() const { return _cameraManager.GetPreviewCamera(); }
	CameraManager &GetCameraManager() { return _cameraManager; }
	Hand *GetHand(uint8_t index) { return _hands.at(index); }

	RN::Model *AssignShader(RN::Model *model, Types::MaterialType materialType) const;
	RN::Model *MakeDeepCopy(RN::Model *model) const;

	void AddLevelNode(RN::SceneNode *node);
	void RemoveLevelNode(RN::SceneNode *node);
	void RemoveAllLevelNodes();

	bool GetIsDash() const { return _isDash; }

	void LoadLevel();

protected:
	void WillBecomeActive() override;
	void DidBecomeActive() override;

	void WillUpdate(float delta) override;

	CameraManager _cameraManager;

	RN::Array *_levelNodes;

	RN::ShaderLibrary *_shaderLibrary;

	RN::VRWindow *_vrWindow;

	RN::JoltWorld *_physicsWorld;

	bool _isPaused;
	bool _isDash;

	std::array<Hand *, 2> _hands;

	static World *_sharedInstance;
};
} // namespace ART
