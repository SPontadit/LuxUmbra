#ifndef VERTEX_H_INCLUDED
#define VERTEX_H_INCLUDED

#include <array>

#include "glm\glm.hpp"

#include "rhi\LuxVkImpl.h"

namespace lux
{

	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 textureCoordinate;
		glm::vec3 normal;

		static VkVertexInputBindingDescription GetBindingDescription() noexcept;
		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() noexcept;

		bool operator==(const Vertex& lhs) const noexcept;
	};

} // namespace lux

#endif // VERTEX_H_INCLUDED