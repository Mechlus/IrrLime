// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#include "CParticleSphereEmitter.h"
#include "os.h"
#include "IAttributes.h"

namespace irr
{
namespace scene
{

//! constructor
CParticleSphereEmitter::CParticleSphereEmitter(
	const core::vector3df& center, f32 radius,
	const core::vector3df& direction, u32 minParticlesPerSecond,
	u32 maxParticlesPerSecond, const video::SColor& minStartColor,
	const video::SColor& maxStartColor, u32 lifeTimeMin, u32 lifeTimeMax,
	s32 maxAngleDegrees,
	const core::dimension2df& minStartSize,
	const core::dimension2df& maxStartSize )
	: Center(center), Radius(radius), Direction(direction),
	MinStartSize(minStartSize), MaxStartSize(maxStartSize),
	MinParticlesPerSecond(minParticlesPerSecond),
	MaxParticlesPerSecond(maxParticlesPerSecond),
	MinStartColor(minStartColor), MaxStartColor(maxStartColor),
	MinLifeTime(lifeTimeMin), MaxLifeTime(lifeTimeMax),
	Time(0), Emitted(0), MaxAngleDegrees(maxAngleDegrees)
{
	#ifdef _DEBUG
	setDebugName("CParticleSphereEmitter");
	#endif

}


//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
s32 CParticleSphereEmitter::emitt(u32 now, u32 timeSinceLastCall, SParticle*& outArray)
{
	Time += timeSinceLastCall;
	Particles.set_used(0);

	u32 particlesToEmit = 0;

	if (Emitted > 0) {
		particlesToEmit = Emitted;
		Emitted = 0;
	}
	else {
		const u32 pps = (MaxParticlesPerSecond - MinParticlesPerSecond);
		const f32 perSecond = pps ? ((f32)MinParticlesPerSecond + os::Randomizer::frand() * pps) : MinParticlesPerSecond;
		const f32 everyWhatMillisecond = 1000.0f / perSecond;

		if (Time > everyWhatMillisecond) {
			particlesToEmit = (u32)((Time / everyWhatMillisecond) + 0.5f);
			Time = 0;

			if (particlesToEmit > MaxParticlesPerSecond * 2)
				particlesToEmit = MaxParticlesPerSecond * 2;
		}
	}

	if (particlesToEmit > 0) {
		SParticle p;

		for (u32 i = 0; i < particlesToEmit; ++i) {
			const f32 distance = os::Randomizer::frand() * Radius;

			p.pos.set(Center + distance);
			p.pos.rotateXYBy(os::Randomizer::frand() * 360.f, Center);
			p.pos.rotateYZBy(os::Randomizer::frand() * 360.f, Center);
			p.pos.rotateXZBy(os::Randomizer::frand() * 360.f, Center);

			p.startTime = now;
			p.vector = Direction;

			if (MaxAngleDegrees) {
				core::vector3df tgt = Direction;
				tgt.rotateXYBy(os::Randomizer::frand() * MaxAngleDegrees);
				tgt.rotateYZBy(os::Randomizer::frand() * MaxAngleDegrees);
				tgt.rotateXZBy(os::Randomizer::frand() * MaxAngleDegrees);
				p.vector = tgt;
			}

			p.endTime = now + MinLifeTime;
			if (MaxLifeTime != MinLifeTime)
				p.endTime += os::Randomizer::rand() % (MaxLifeTime - MinLifeTime);

			if (MinStartColor == MaxStartColor)
				p.color = MinStartColor;
			else
				p.color = MinStartColor.getInterpolated(MaxStartColor, os::Randomizer::frand());

			p.startColor = p.color;
			p.startVector = p.vector;

			if (MinStartSize == MaxStartSize)
				p.startSize = MinStartSize;
			else
				p.startSize = MinStartSize.getInterpolated(MaxStartSize, os::Randomizer::frand());
			p.size = p.startSize;

			Particles.push_back(p);
		}

		outArray = Particles.pointer();
		return Particles.size();
	}

	return 0;
}

//! Writes attributes of the object.
void CParticleSphereEmitter::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	out->addVector3d("Center", Direction);
	out->addFloat("Radius", Radius);

	out->addVector3d("Direction", Direction);
	out->addFloat("MinStartSizeWidth", MinStartSize.Width);
	out->addFloat("MinStartSizeHeight", MinStartSize.Height);
	out->addFloat("MaxStartSizeWidth", MaxStartSize.Width);
	out->addFloat("MaxStartSizeHeight", MaxStartSize.Height); 
	out->addInt("MinParticlesPerSecond", MinParticlesPerSecond);
	out->addInt("MaxParticlesPerSecond", MaxParticlesPerSecond);
	out->addColor("MinStartColor", MinStartColor);
	out->addColor("MaxStartColor", MaxStartColor);
	out->addInt("MinLifeTime", MinLifeTime);
	out->addInt("MaxLifeTime", MaxLifeTime);
	out->addInt("MaxAngleDegrees", MaxAngleDegrees);
}

//! Reads attributes of the object.
void CParticleSphereEmitter::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	Center = in->getAttributeAsVector3d("Center");
	Radius = in->getAttributeAsFloat("Radius");

	Direction = in->getAttributeAsVector3d("Direction");
	if (Direction.getLength() == 0)
		Direction.set(0,0.01f,0);

	int idx = -1;
	idx = in->findAttribute("MinStartSizeWidth");
	if ( idx >= 0 )
		MinStartSize.Width = in->getAttributeAsFloat(idx);
	idx = in->findAttribute("MinStartSizeHeight");
	if ( idx >= 0 )
		MinStartSize.Height = in->getAttributeAsFloat(idx);
	idx = in->findAttribute("MaxStartSizeWidth");
	if ( idx >= 0 )
		MaxStartSize.Width = in->getAttributeAsFloat(idx);
	idx = in->findAttribute("MaxStartSizeHeight");
	if ( idx >= 0 )
		MaxStartSize.Height = in->getAttributeAsFloat(idx); 
	MinParticlesPerSecond = in->getAttributeAsInt("MinParticlesPerSecond");
	MaxParticlesPerSecond = in->getAttributeAsInt("MaxParticlesPerSecond");

	MinParticlesPerSecond = core::max_(1u, MinParticlesPerSecond);
	MaxParticlesPerSecond = core::max_(MaxParticlesPerSecond, 1u);
	MaxParticlesPerSecond = core::min_(MaxParticlesPerSecond, 200u);
	MinParticlesPerSecond = core::min_(MinParticlesPerSecond, MaxParticlesPerSecond);

	MinStartColor = in->getAttributeAsColor("MinStartColor");
	MaxStartColor = in->getAttributeAsColor("MaxStartColor");
	MinLifeTime = in->getAttributeAsInt("MinLifeTime");
	MaxLifeTime = in->getAttributeAsInt("MaxLifeTime");
	MinLifeTime = core::max_(0u, MinLifeTime);
	MaxLifeTime = core::max_(MaxLifeTime, MinLifeTime);
	MinLifeTime = core::min_(MinLifeTime, MaxLifeTime);

	MaxAngleDegrees = in->getAttributeAsInt("MaxAngleDegrees");
}

} // end namespace scene
} // end namespace irr

