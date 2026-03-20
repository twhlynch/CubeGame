#pragma once

#include <RNJoltShape.h>
#include <Rayne.h>

namespace CG
{

class PhysicsObject : public RN::Entity
{
public:
	PhysicsObject(RN::Model *model);
	virtual RN::JoltShape *CreateShape() const = 0;

private:
	RNDeclareMeta(PhysicsObject);
};

class PhysicsCube : public PhysicsObject
{
public:
	PhysicsCube(RN::Model *model);
	RN::JoltShape *CreateShape() const override;

private:
	RNDeclareMeta(PhysicsCube);
};

class PhysicsSphere : public PhysicsObject
{
public:
	PhysicsSphere(RN::Model *model);
	RN::JoltShape *CreateShape() const override;

private:
	RNDeclareMeta(PhysicsSphere);
};

class PhysicsPyramid : public PhysicsObject
{
public:
	PhysicsPyramid(RN::Model *model);
	RN::JoltShape *CreateShape() const override;

private:
	RNDeclareMeta(PhysicsPyramid);
};

} // namespace CG
