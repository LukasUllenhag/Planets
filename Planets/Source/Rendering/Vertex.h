#pragma once

struct TightlyPackedVector3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

struct Uv
{
	float u = 0.0f;
	float v = 0.0f;
};

struct Vertex
{
	// The reason why we can not use the normal vector class, is
	// because it inherits from "ContainerBase". Two instances, of the 
	// normal vector class, stored next to each other in memory will, 
	// thanks to the inheritance, have padding between their data.
	TightlyPackedVector3 position;
	Uv uv;
	TightlyPackedVector3 normal;
};