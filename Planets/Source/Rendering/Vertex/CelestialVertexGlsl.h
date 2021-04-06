#pragma once
#include "Source/Mathematics/Vector/TightlyPacked/TightlyPackedVector3.h"

// Celestial vertex that is aligned according to the std430 storage layout
struct CelestialVertexGlsl
{
	TightlyPackedVector3 alignas(4 * 4) position;
	TightlyPackedVector3 alignas(4 * 4) uv;
	TightlyPackedVector3 alignas(4 * 4) normal;
};