#include "scene\LightNode.h"

namespace lux::scene
{
	using namespace lux;

	LightNode::LightNode(Node* parent, LightType type, glm::vec3 color, int16_t shadowMappingResourceIndex) noexcept
		: Node(parent), type(type), color(color), shadowMappingResourceIndex(shadowMappingResourceIndex)
	{

	}

	LightNode::LightNode(Node* parent, glm::vec3 position, glm::vec3 rotation, LightType type, glm::vec3 color, int16_t shadowMappingResourceIndex) noexcept
		: Node(parent, position, rotation), type(type), color(color), shadowMappingResourceIndex(shadowMappingResourceIndex)
	{

	}

	LightType LightNode::GetType() const noexcept
	{
		return type;
	}

	void LightNode::SetColor(glm::vec3 newColor) noexcept
	{
		color = newColor;
	}

	glm::vec3 LightNode::GetColor() const noexcept
	{
		return color;
	}

	int16_t LightNode::GetShadowMappingResourceIndex() const noexcept
	{
		return shadowMappingResourceIndex;
	}

} // namespace lux::scene
