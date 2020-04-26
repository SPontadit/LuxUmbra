#include "AABB.h"

namespace lux
{

	AABB::AABB() noexcept
		: min(0.f), max(0.f)
	{

	}

	AABB& AABB::Transform(const glm::mat4& transform) noexcept
	{
		glm::vec3 oldMin = min, oldMax = max;
		glm::vec3 translation = transform[3].xyz;

		glm::mat3 rot = glm::mat3(transform);
		rot[2] *= -1.f;

		for (int i = 0; i < 3; i++)
		{
			min[i] = max[i] = translation[i];

			for (int j = 0; j < 3; j++)
			{
				float e = rot[i][j] * oldMin[j];
				float f = rot[i][j] * oldMax[j];

				if (e < f)
				{
					min[i] += e;
					max[i] += f;
				}
				else
				{
					min[i] += f;
					max[i] += e;
				}
			}
		}

		return *this;
	}

	void AABB::MakeFit(const AABB& other) noexcept
	{
		MakeFit(other.min, other.max);
	}

	void AABB::MakeFit(const glm::vec3& newMin, const glm::vec3& newMax) noexcept
	{
		min = glm::min(min, newMin);
		max = glm::max(max, newMax);
	}

}