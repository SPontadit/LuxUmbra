#include "AABB.h"

namespace lux
{

	AABB::AABB() noexcept
		: min(0.f), max(0.f)
	{

	}

	AABB& AABB::Transform(glm::mat4 transform) noexcept
	{
		min = (transform * glm::vec4(min, 1.0f)).xyz;
		max = (transform * glm::vec4(max, 1.0f)).xyz;

		return *this;
	}

	void AABB::MakeFit(glm::vec3 newMin, glm::vec3 newMax) noexcept
	{
		if (newMin.x < min.x)
			min.x = newMin.x;
		if (newMin.y < min.y)
			min.y = newMin.y;
		if (newMin.z < min.z)
			min.z = newMin.z;

		if (newMax.x > max.x)
			max.x = newMax.x;
		if (newMax.y > max.y)
			max.y = newMax.y;
		if (newMax.z > max.z)
			max.z = newMax.z;
	}

}