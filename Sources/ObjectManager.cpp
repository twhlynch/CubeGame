#include "ObjectManager.hpp"

#include <Rendering/RNMaterial.h>

#include <RayneConfig.h>

#include "MeshHelpers.hpp"
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
		RN::Color(1.0f, 0.0f, 0.0f), // red
		RN::Color(0.0f, 1.0f, 0.0f), // blue
		RN::Color(0.0f, 0.0f, 1.0f), // green
		RN::Color(1.0f, 1.0f, 0.0f), // yellow
		RN::Color(1.0f, 0.2f, 0.0f), // orange
		RN::Color(0.5f, 0.0f, 1.0f), // purple
		RN::Color(1.0f, 1.0f, 1.0f), // white
		RN::Color(0.2f, 0.2f, 0.2f), // gray
		RN::Color(0.0f, 0.0f, 0.0f), // black
	};

	_meshes = {
		Mesh::CubeMesh()->Retain(),
		Mesh::SphereMesh()->Retain(),
		Mesh::PyramidMesh()->Retain(),
		Mesh::RectangularPrismMesh()->Retain(),
		Mesh::CylinderMesh()->Retain(),
	};

	for (auto &mesh : _meshes)
	{
		for (const auto &diffuse : colors)
		{
			auto *material = RN::Material::WithShaders(nullptr, nullptr);
			material->SetDiffuseColor(diffuse);
			auto *model = world->AssignShader(new RN::Model(mesh, material), Types::MaterialType::MaterialDefault);
			_models->AddObject(model->Autorelease());
		}
	}
}

ObjectManager::~ObjectManager()
{
	_models->Release();

	for (auto &mesh : _meshes)
	{
		mesh->Autorelease();
	}
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

	if (index >= GetColorCount() * 2 && index < GetColorCount() * 3)
	{
		auto *object = new PhysicsPyramid(GetModelWithIndex(index));
		return new PhysicsGroup(object);
	}

	if (index >= GetColorCount() * 3 && index < GetColorCount() * 4)
	{
		auto *object = new PhysicsRectangularPrism(GetModelWithIndex(index));
		return new PhysicsGroup(object);
	}

	if (index >= GetColorCount() * 4 && index < GetColorCount() * 5)
	{
		auto *object = new PhysicsCylinder(GetModelWithIndex(index));
		return new PhysicsGroup(object);
	}

	// and just in case
	auto *object = new PhysicsCube(GetModelWithIndex(index));
	return new PhysicsGroup(object);
}

RN::Mesh *ObjectManager::GetMeshWithIndex(size_t index)
{
	RN_DEBUG_ASSERT(index < GetShapeCount(), "Invalid mesh index");

	return _meshes[index];
}

} // namespace CG
