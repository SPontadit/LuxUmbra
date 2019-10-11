#include "Vertex.h"

namespace lux
{

	VkVertexInputBindingDescription Vertex::GetBindingDescription() noexcept
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	
		return bindingDescription;
	}

	std::array<VkVertexInputAttributeDescription, 3> Vertex::GetAttributeDescriptions() noexcept
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescription = {};

		attributeDescription[0].binding = 0;
		attributeDescription[0].location = 0;
		attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[0].offset = offsetof(Vertex, position);

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, textureCoordinate);

		attributeDescription[2].binding = 0;
		attributeDescription[2].location = 2;
		attributeDescription[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[2].offset = offsetof(Vertex, normal);

		return attributeDescription;
	}

	bool Vertex::operator==(const Vertex& lhs) const noexcept
	{
		return position == lhs.position && textureCoordinate == lhs.textureCoordinate && normal == lhs.normal;
	}

} // namespace lux