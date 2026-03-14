#include "World.hpp"

#include <RNUI.h>

#include "HandTrackingInfo.hpp"

namespace CG
{
World *World::_sharedInstance = nullptr;

World *World::GetSharedInstance()
{
	return _sharedInstance;
}

void World::Exit()
{
	if (_sharedInstance)
	{
		_sharedInstance->RemoveAttachment(_sharedInstance->_physicsWorld);

#if RN_PLATFORM_ANDROID
		if (_sharedInstance->_vrWindow)
		{
			_sharedInstance->_vrWindow->Release(); // Reference from VRCamera
			_sharedInstance->_vrWindow->Release(); // Reference from World
			_sharedInstance->_vrWindow->Release(); // Reference from Application
												   //_sharedInstance->_vrWindow->StopRendering();
		}
#endif
	}

	exit(0);
	// RN::Kernel::GetSharedInstance()->Exit();
}

World::World(RN::VRWindow *vrWindow) : _vrWindow(nullptr), _physicsWorld(nullptr), _isPaused(false), _isDash(false), _shaderLibrary(nullptr), _hands({nullptr, nullptr})
{
	_sharedInstance = this;

	if (vrWindow)
	{
		_vrWindow = vrWindow->Retain();
	}

	_levelNodes = new RN::Array();
}

World::~World()
{
	SafeRelease(_hands.at(0));
	SafeRelease(_hands.at(1));
}

void World::WillBecomeActive()
{
	RN::Scene::WillBecomeActive();

	if (!RN::Renderer::IsHeadless())
	{
		RN::Renderer *activeRenderer = RN::Renderer::GetActiveRenderer();
		_shaderLibrary = activeRenderer->CreateShaderLibraryWithFile(RNCSTR("shaders/Shaders.json"));
	}

	_cameraManager.Setup(_vrWindow);

	_physicsWorld = new RN::JoltWorld(RN::Vector3(0.0f, -9.81f, 0.0f));
	AddAttachment(_physicsWorld->Autorelease());

	LoadLevel();
}

void World::DidBecomeActive()
{
	RN::SceneBasic::DidBecomeActive();
	_cameraManager.SetCameraAmbientColor(RN::Color::White(), 1.0f, nullptr);
}

void World::WillUpdate(float delta)
{
	RN::Scene::WillUpdate(delta);

	_isPaused = false;
	_isDash = false;
	RN::VRHMDTrackingState::Mode headsetState = _cameraManager.Update(delta);
	if (headsetState == RN::VRHMDTrackingState::Mode::Paused)
	{
		_isPaused = true;
		_isDash = true;
	}
	else if (headsetState == RN::VRHMDTrackingState::Mode::Disconnected)
	{
		Exit();
	}

	if (RN::InputManager::GetSharedInstance()->IsControlToggling(RNCSTR("ESC")))
	{
		Exit();
	}

	if (RN::InputManager::GetSharedInstance()->IsControlToggling(RNCSTR("SPACE")))
	{
		static auto *model = AssignShader(RN::Model::WithCube(RN::Color::Red()), Types::MaterialType::MaterialDefault);
		auto *cube = new PhysicsCube(model);
		cube->SetPosition({0, 1, 0});
		AddLevelNode(cube->Autorelease());
	}
}

RN::Model *World::AssignShader(RN::Model *model, Types::MaterialType materialType) const
{
	if (RN::Renderer::IsHeadless())
	{
		return model;
	}

	World *world = World::GetSharedInstance();
	RN::ShaderLibrary *shaderLibrary = world->GetShaderLibrary();

	RN::Model::LODStage *lodStage = model->GetLODStage(0);
	for (int i = 0; i < lodStage->GetCount(); i++)
	{
		RN::Material *material = lodStage->GetMaterialAtIndex(i);

		switch (materialType)
		{
			case Types::MaterialType::MaterialDefault:
				{
					material->SetDepthWriteEnabled(true);
					material->SetDepthMode(RN::DepthMode::GreaterOrEqual);
					material->SetAlphaToCoverage(false);
					material->SetAmbientColor(RN::Color::White());
					RN::Shader::Options *shaderOptions = RN::Shader::Options::WithMesh(lodStage->GetMeshAtIndex(i));
					material->SetVertexShader(shaderLibrary->GetShaderWithName(RNCSTR("main_vertex"), shaderOptions));
					material->SetFragmentShader(shaderLibrary->GetShaderWithName(RNCSTR("main_fragment"), shaderOptions));
					shaderOptions->EnableMultiview();
					material->SetVertexShader(GetShaderLibrary()->GetShaderWithName(RNCSTR("main_vertex"), shaderOptions), RN::Shader::UsageHint::Multiview);
					material->SetFragmentShader(GetShaderLibrary()->GetShaderWithName(RNCSTR("main_fragment"), shaderOptions), RN::Shader::UsageHint::Multiview);
					break;
				}
		}
	}

	return model;
}

RN::Model *World::MakeDeepCopy(RN::Model *model) const
{
	RN::Model *result = model->Copy();

	RN::Model::LODStage *lodStage = result->GetLODStage(0);
	for (int i = 0; i < lodStage->GetCount(); i++)
	{
		RN::Material *material = lodStage->GetMaterialAtIndex(i)->Copy();
		lodStage->ReplaceMaterial(material->Autorelease(), i);
	}

	return result->Autorelease();
}

void World::AddLevelNode(RN::SceneNode *node)
{
	_levelNodes->AddObject(node);
	AddNode(node);
}

void World::RemoveLevelNode(RN::SceneNode *node)
{
	_levelNodes->RemoveObject(node);
	RemoveNode(node);
}

void World::RemoveAllLevelNodes()
{
	_levelNodes->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
		RemoveNode(node);
	});

	_levelNodes->RemoveAllObjects();
}

void World::LoadLevel()
{
	RemoveAllLevelNodes();

	// ground plane
	auto *groundModel = AssignShader(RN::Model::WithCube(RN::Color::ClearColor()), Types::MaterialType::MaterialDefault);
	auto *groundEntity = new RN::Entity(groundModel);
	groundEntity->SetScale(RN::Vector3(50.0f, 0.1f, 50.0f));
	groundEntity->SetPosition(RN::Vector3(0.0f, 0.0f, 0.0f));

	auto *groundMaterial = new RN::JoltMaterial();
	auto *groundShape = RN::JoltCompoundShape::WithModel(groundModel, groundMaterial->Autorelease(), groundEntity->GetScale(), true);
	auto *groundBody = RN::JoltStaticBody::WithShape(groundShape);
	groundBody->SetCollisionFilter(Types::CollisionLevel, Types::CollisionAll);

	groundEntity->AddAttachment(groundBody);
	AddLevelNode(groundEntity->Autorelease());

	// hands
	if (!_hands.at(0)) { _hands.at(0) = new Hand(0); }
	if (!_hands.at(1)) { _hands.at(1) = new Hand(1); }

	AddLevelNode(_hands.at(0));
	AddLevelNode(_hands.at(1));

	// hand tracking info
	auto *handTrackingInfo = new HandTrackingInfo();
	AddLevelNode(handTrackingInfo->Autorelease());

	if (!RN::Renderer::IsHeadless())
	{
		// sky
		auto *skyMesh = RN::Mesh::WithColoredCube(100, RN::Color::White());
		auto *skyMaterial = RN::Material::WithShaders(nullptr, nullptr);
		skyMaterial->SetDepthMode(RN::DepthMode::GreaterOrEqual);
		skyMaterial->SetCullMode(RN::CullMode::None);

		auto *skyShaderOptions = RN::Shader::Options::WithMesh(skyMesh);
		skyShaderOptions->AddDefine(RNCSTR("RN_SKY"), RNCSTR("1"));
		skyMaterial->SetVertexShader(GetShaderLibrary()->GetShaderWithName(RNCSTR("sky_vertex"), skyShaderOptions));
		skyMaterial->SetFragmentShader(GetShaderLibrary()->GetShaderWithName(RNCSTR("sky_fragment"), skyShaderOptions));
		skyMaterial->SetVertexShader(RN::Renderer::GetActiveRenderer()->GetDefaultShader(RN::Shader::Type::Vertex, skyShaderOptions, RN::Shader::UsageHint::Depth), RN::Shader::UsageHint::Depth);
		skyMaterial->SetFragmentShader(RN::Renderer::GetActiveRenderer()->GetDefaultShader(RN::Shader::Type::Fragment, skyShaderOptions, RN::Shader::UsageHint::Depth), RN::Shader::UsageHint::Depth);
		skyShaderOptions->EnableMultiview();
		skyMaterial->SetVertexShader(GetShaderLibrary()->GetShaderWithName(RNCSTR("sky_vertex"), skyShaderOptions), RN::Shader::UsageHint::Multiview);
		skyMaterial->SetFragmentShader(GetShaderLibrary()->GetShaderWithName(RNCSTR("sky_fragment"), skyShaderOptions), RN::Shader::UsageHint::Multiview);

		auto *skyModel = new RN::Model(skyMesh, skyMaterial);
		auto *skyEntity = new RN::Entity(skyModel->Autorelease());
		skyEntity->SetScale(RN::Vector3(10.0f));
		skyEntity->SetRenderPriority(RN::SceneNode::RenderPriority::RenderSky);
		AddLevelNode(skyEntity->Autorelease());
	}

	_physicsWorld->SetPaused(false);
}
} // namespace CG
