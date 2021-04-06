#pragma once
#include "Source/Mathematics/Vector/TightlyPacked/TightlyPackedVector2.h"
#include "Source/Mathematics/Vector/TightlyPacked/TightlyPackedVector3.h"

// Vertex that is aligned according to the std430 storage layout
struct VertexGlsl
{
	TightlyPackedVector3 alignas(4 * 4) position;
	TightlyPackedVector2 alignas(2 * 4) uv;
	TightlyPackedVector3 alignas(4 * 4) normal;
};