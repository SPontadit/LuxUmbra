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
		glm::vec3 tangent;
		glm::vec3 bitangent;

		static VkVertexInputBindingDescription GetBindingDescription() noexcept;
		static VkVertexInputAttributeDescription GetPositionOnlyAttributeDescription() noexcept;
		static std::array<VkVertexInputAttributeDescription, 3> GetBasicAttributeDescriptions() noexcept;
		static std::array<VkVertexInputAttributeDescription, 5> GetFullAttributeDescriptions() noexcept;

		bool operator==(const Vertex& lhs) const noexcept;
	};

	enum class VertexLayout : uint32_t
	{
		VERTEX_POSITION_ONLY_LAYOUT = 0,
		VERTEX_BASIC_LAYOUT,
		VERTEX_FULL_LAYOUT,
		NO_VERTEX_LAYOUT,
		VERTEX_LAYOUT_COUNT
	};

} // namespace lux

#endif // VERTEX_H_INCLUDED