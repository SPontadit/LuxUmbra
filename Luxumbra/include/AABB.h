#ifndef AABB_H_INCLUDED
#define AABB_H_INCLUDED

#include "Luxumbra.h"

#include "glm\glm.hpp"

namespace lux
{

	struct AABB
	{
		AABB() noexcept;
		AABB(const AABB&) noexcept = default;
		AABB(AABB&&) noexcept = default;

		~AABB() noexcept = default;

		AABB& operator =(const AABB&) noexcept = default;
		AABB& operator =(AABB&&) noexcept = default;

		glm::vec3 min, max;

		AABB& Transform(const glm::mat4& transform) noexcept;
		void MakeFit(const AABB& other) noexcept;
		void MakeFit(const glm::vec3& newMin, const glm::vec3& newMax) noexcept;
	};

} // namespace lux

#endif // AABB_H_INCLUDED