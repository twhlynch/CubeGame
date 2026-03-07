#include "PartsPicker.hpp"

#include "Types.hpp"
#include "World.hpp"

namespace ART
{

static constexpr float scale = 0.007f;
static constexpr float gap = 0.03f;
static constexpr float handOffset = 0.06f;

PartsPicker::PartsPicker()
{
	World *world = World::GetSharedInstance();

	std::array<RN::Color, count> colors = {
		RN::Color::Red(),
		RN::Color::Green(),
		RN::Color::Blue(),
		RN::Color::Yellow(),
		RN::Color::Black(),
		RN::Color::White(),
	};

	static auto *material = new RN::JoltMaterial();
	auto *shape = RN::JoltBoxShape::WithHalfExtents(scale, material->Autorelease(), scale);

	constexpr float centerOffset = ((static_cast<float>(count) / 2.0f) * gap) - (gap / 2.0f);

	for (size_t i = 0; i < count; ++i)
	{
		RN::Vector3 pos(0, handOffset, (static_cast<float>(i) * gap) - centerOffset);

		auto *model = world->AssignShader(RN::Model::WithCube(colors.at(i)), Types::MaterialType::MaterialDefault);
		auto *entity = new RN::Entity(model);
		entity->SetScale(scale);
		entity->SetPosition(pos);

		auto *physicsBody = RN::JoltStaticBody::WithShape(shape);
		physicsBody->SetCollisionFilter(Types::CollisionPartPicker, Types::CollisionTest);
		entity->AddAttachment(physicsBody);
		_cubes.at(i) = entity;

		AddChild(entity->Autorelease());
	}
}

void PartsPicker::SetHidden(bool hidden)
{
	if (hidden == _isHidden) { return; }
	_isHidden = hidden;

	if (hidden)
	{
		AddFlags(RN::SceneNode::Flags::Hidden);
		for (auto *cube : _cubes)
		{
			cube->AddFlags(RN::SceneNode::Flags::Hidden);
		}
	}
	else
	{
		RemoveFlags(RN::SceneNode::Flags::Hidden);
		for (auto *cube : _cubes)
		{
			cube->RemoveFlags(RN::SceneNode::Flags::Hidden);
		}
	}
}

} // namespace ART
