#pragma once

#include <Scene/RNEntity.h>

namespace CG
{

class Part : public RN::Entity
{
public:
	Part(RN::Model *model);

	void SetIndex(size_t index);
	size_t GetIndex() const;

private:
	size_t _index;

	RNDeclareMeta(Part);
};

} // namespace CG
