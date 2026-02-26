#pragma once

#include <Scene/RNEntity.h>

#include <Rayne.h>

namespace ART
{

class Hand : public RN::SceneNode
{
public:
	Hand(uint8_t index);

	void Update(float delta) override;

	[[nodiscard]] bool IsPinching() const { return _pinching; }

private:
	uint8_t _handIndex;

	bool _pinching;
	bool _wasPinching;

	RN::Entity _entity;
};

} // namespace ART
