#include "ObjectManager.hpp"

#include <Rendering/RNMaterial.h>

#include <RayneConfig.h>

#include "MeshHelpers.hpp"
#include "Types.hpp"
#include "World.hpp"

namespace CG
{

static constexpr size_t ColorCount = 9;

static const ColorProperties ColorPropertiesLookup[ColorCount] = {
	{.mass = 1.0f, .friction = 0.50f, .restitution = 0.10f, .gravity = 1.0f}, // 0 Red    Normal
	{.mass = 1.0f, .friction = 0.50f, .restitution = 0.10f, .gravity = 1.0f}, // 1 Green  Normal
	{.mass = 1.0f, .friction = 0.05f, .restitution = 0.10f, .gravity = 1.0f}, // 2 Blue   Ice
	{.mass = 0.2f, .friction = 0.50f, .restitution = 0.20f, .gravity = 1.0f}, // 3 Yellow Light
	{.mass = 1.0f, .friction = 0.95f, .restitution = 0.00f, .gravity = 1.0f}, // 4 Orange Grip
	{.mass = 1.0f, .friction = 0.50f, .restitution = 0.85f, .gravity = 1.0f}, // 5 Purple Bouncy
	{.mass = 1.0f, .friction = 0.50f, .restitution = 0.10f, .gravity = 1.0f}, // 6 White  Normal
	{.mass = 4.0f, .friction = 0.70f, .restitution = 0.10f, .gravity = 1.0f}, // 7 Grey   Heavy
	{.mass = 1.0f, .friction = 0.50f, .restitution = 0.10f, .gravity = 0.2f}, // 8 Black  Floaty
};

const ColorProperties &ObjectManager::GetColorProperties(size_t colorIndex)
{
	return ColorPropertiesLookup[colorIndex];
}

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
		RN::Mesh::WithColoredCube(RN::Vector3(1.0f, 1.0f, 1.0f), RN::Color::White())->Retain(),
		Mesh::SphereMesh()->Retain(),
		Mesh::PyramidMeshWithNormals()->Retain(),
		RN::Mesh::WithColoredCube(RN::Vector3(0.5f, 1.0f, 0.5f), RN::Color::White())->Retain(),
		Mesh::CylinderMeshWithNormals()->Retain(),
	};

	_physicsMeshes = {
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

	PhysicsObject *object = nullptr;

	if (index < GetColorCount())
	{
		object = new PhysicsCube(GetModelWithIndex(index));
	}
	else if (index < GetColorCount() * 2)
	{
		object = new PhysicsSphere(GetModelWithIndex(index));
	}
	else if (index < GetColorCount() * 3)
	{
		object = new PhysicsPyramid(GetModelWithIndex(index));
	}
	else if (index < GetColorCount() * 4)
	{
		object = new PhysicsRectangularPrism(GetModelWithIndex(index));
	}
	else
	{
		object = new PhysicsCylinder(GetModelWithIndex(index));
	}

	object->SetSourceIndex(index);
	auto *group = new PhysicsGroup(object);
	return group;
}

RN::Mesh *ObjectManager::GetMeshWithIndex(size_t index)
{
	RN_DEBUG_ASSERT(index < GetShapeCount(), "Invalid mesh index");

	return _meshes[index];
}

RN::Mesh *ObjectManager::GetPhysicsMeshWithIndex(size_t index)
{
	RN_DEBUG_ASSERT(index < GetShapeCount(), "Invalid mesh index");

	return _physicsMeshes[index];
}

} // namespace CG
