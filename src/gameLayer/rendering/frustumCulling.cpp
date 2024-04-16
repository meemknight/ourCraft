#include "rendering/frustumCulling.h"
#include <algorithm>
//https://github.com/maritim/LiteEngine/blob/master/Engine/Core/Intersections/Intersection.cpp

bool CheckFrustumVsAABB(const FrustumVolume &frustum, const AABBVolume &boundingBox)
{
	for (std::size_t i = 0; i < FrustumVolume::PLANESCOUNT; i++)
	{
		// this is the current plane
		const glm::dvec4 &plane = frustum.plane[i];

		// p-vertex selection
		const float px = std::signbit(plane.x) ? boundingBox.minVertex.x : boundingBox.maxVertex.x;
		const float py = std::signbit(plane.y) ? boundingBox.minVertex.y : boundingBox.maxVertex.y;
		const float pz = std::signbit(plane.z) ? boundingBox.minVertex.z : boundingBox.maxVertex.z;

		// dot product
		// project p-vertex on plane normal
		// (How far is p-vertex from the origin)
		const float dp = (plane.x * px) + (plane.y * py) + (plane.z * pz);

		// doesn't intersect if it is behind the plane
		if (dp < -plane.w)
		{
			return false;
		}
	}


	return true;
}


bool CheckRayVsAABB(const RayPrimitive &ray, const AABBVolume &aabb, float &distance)
{
	float tMin, tMax;

	glm::dvec3 invRayDir = glm::dvec3(1.0) / ray.direction;
	glm::dvec3 t1 = (aabb.minVertex - ray.origin) * invRayDir;
	glm::dvec3 t2 = (aabb.maxVertex - ray.origin) * invRayDir;

	glm::dvec3 tmin = glm::min(t1, t2);
	glm::dvec3 tmax = glm::max(t1, t2);

	tMin = glm::max(tmin.x, glm::max(tmin.y, tmin.z));
	tMax = glm::min(tmax.x, glm::min(tmax.y, tmax.z));

	if (tMax > tMin)
	{
		distance = tMin;
	}

	return tMax > tMin;
}
