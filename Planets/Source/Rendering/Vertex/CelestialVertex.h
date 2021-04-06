#pragma once
#include "Source/Mathematics/Vector/TightlyPacked/TightlyPackedVector3.h"

struct CelestialVertex
{
	// The reason why we can not use the normal vector class, is
	// because it inherits from "ContainerBase". Two instances of the 
	// normal vector class, when stored next to each other in memory will, 
	// thanks to the inheritance, have undesirable padding between their data.
	TightlyPackedVector3 position;
	// For the celestial vertex we use a 3-dimensional vector for the
	// UV coordinate, instead of a 2-dimensional one. Only the vertices
	// inside some of the craters should have a texture applied to them, and
	// the additional coordinate signals whether or not the vertex is inside
	// the texture.
	TightlyPackedVector3 uv;
	TightlyPackedVector3 normal;
};