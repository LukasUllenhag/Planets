#pragma once
#include "Source/Mathematics/Vector/TightlyPacked/TightlyPackedVector2.h"
#include "Source/Mathematics/Vector/TightlyPacked/TightlyPackedVector3.h"

struct Vertex
{
	// The reason why we can not use the normal vector class, is
	// because it inherits from "ContainerBase". Two instances of the 
	// normal vector class, when stored next to each other in memory will, 
	// thanks to the inheritance, have undesirable padding between their data.
	TightlyPackedVector3 position;
	TightlyPackedVector2 uv;
	TightlyPackedVector3 normal;
};