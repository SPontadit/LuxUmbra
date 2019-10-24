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

		for (int i = 0; i < 3; i++)
		{
			min[i] = max[i] = translation[i];

			for (int j = 0; j < 3; j++)
			{
				float e = transform[i][j] * oldMin[j];
				float f = transform[i][j] * oldMax[j];

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