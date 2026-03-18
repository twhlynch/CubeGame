#include "ObjectManager.hpp"

#include <Rendering/RNMaterial.h>

#include <RayneConfig.h>

#include "Types.hpp"
#include "World.hpp"

namespace CG
{

ObjectManager::ObjectManager()
	: _models(new RN::Array())
{
	World *world = World::GetSharedInstance();

	constexpr float scale = 1.0f;
	constexpr float segments = 12;
	const auto color = RN::Color::White();

	std::array<RN::Color, GetColorCount()> colors = {
		RN::Color::Red(),
		RN::Color::Green(),
		RN::Color::Blue(),
		RN::Color::Yellow(),
		RN::Color::Black(),
		RN::Color::White(),
	};

	std::array<RN::Mesh *, GetShapeCount()> meshes = {
		RN::Mesh::WithColoredCube(scale, color),
		RN::Mesh::WithSphereMesh(scale, segments, segments, color),
	};

	for (auto &mesh : meshes)
	{
		for (const auto &diffuse : colors)
		{
			auto *material = RN::Material::WithShaders(nullptr, nullptr);
			material->SetDiffuseColor(diffuse);
			material->SetCullMode(RN::CullMode::None); // FIXME: fix Rayne Sphere mesh being inside out
			auto *model = world->AssignShader(new RN::Model(mesh, material), Types::MaterialType::MaterialDefault);
			_models->AddObject(model->Autorelease());
		}
	}
}

ObjectManager::~ObjectManager()
{
	_models->Release();
}

RN::Model *ObjectManager::GetModelWithIndex(size_t index)
{
	RN_DEBUG_ASSERT(index < GetColorCount() * GetShapeCount(), "Invalid object index");

	return _models->GetObjectAtIndex<RN::Model>(index);
}

Part *ObjectManager::CreatePartWithIndex(size_t index)
{
	RN_DEBUG_ASSERT(index < GetColorCount() * GetShapeCount(), "Invalid object index");

	auto *model = GetModelWithIndex(index);
	auto *part = new Part(model);
	return part->Autorelease();
}

PhysicsGroup *ObjectManager::CreatePhysicsObjectWithIndex(size_t index)
{
	RN_DEBUG_ASSERT(index < GetColorCount() * GetShapeCount(), "Invalid object index");

	if (index >= 0 && index < GetColorCount())
	{
		auto *object = new PhysicsCube(GetModelWithIndex(index));
		return new PhysicsGroup(object);
	}

	if (index >= GetColorCount() && index < GetColorCount() * 2)
	{
		auto *object = new PhysicsSphere(GetModelWithIndex(index));
		return new PhysicsGroup(object);
	}

	// and just in case
	auto *object = new PhysicsCube(GetModelWithIndex(index));
	return new PhysicsGroup(object);
}

} // namespace CG
