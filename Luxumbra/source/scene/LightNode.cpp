#include "scene\LightNode.h"

namespace lux::scene
{
	using namespace lux;

	LightNode::LightNode(Node* parent, LightType type, glm::vec3 color) noexcept
		: Node(parent), type(type), color(color), volumeInfo(type)
	{

	}

	LightNode::LightNode(Node* parent, glm::vec3 position, glm::vec3 rotation, LightType type, glm::vec3 color) noexcept
		: Node(parent, position, rotation), type(type), color(color), volumeInfo(type)
	{

	}

	LightNode::VolumeInfo::VolumeInfo(LightType type) noexcept
	{
		switch (type)
		{
		case LightType::LIGHT_TYPE_DIRECTIONAL:
			directional = Directional();
			break;

		case LightType::LIGHT_TYPE_POINT:
			point = Point();
			break;
		}
	}

	LightNode::VolumeInfo::Directional::Directional() noexcept
		: viewPos(0.f, 0.f, 0.f), left(0.f), right(0.f), top(0.f), bottom(0.f),
		nearDist(0.f), farDist(0.f)
	{

	}

	LightNode::VolumeInfo::Point::Point() noexcept
		: radius(0.f), attenuation(0.f)
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

	glm::mat4 LightNode::GetViewTransform() const noexcept
	{
		switch (type)
		{
		case LightType::LIGHT_TYPE_DIRECTIONAL:
			glm::vec3 eyePos = volumeInfo.directional.viewPos;
			glm::vec3 center = eyePos + glm::rotate(GetWorldRotation(), glm::vec3(0.f, 0.f, -1.f));
			return glm::lookAt(eyePos, center, glm::vec3(0.f, 1.f, 0.f));

		case LightType::LIGHT_TYPE_POINT:
		default:
			ASSERT(false);
			return glm::identity<glm::mat4>();
		}
	}

	glm::mat4 LightNode::GetProjectionTransform() const noexcept
	{
		switch (type)
		{
		case LightType::LIGHT_TYPE_DIRECTIONAL:
			return glm::ortho(
				volumeInfo.directional.left,
				volumeInfo.directional.right,
				volumeInfo.directional.bottom,
				volumeInfo.directional.top,
				volumeInfo.directional.nearDist,
				volumeInfo.directional.farDist
			);

		case LightType::LIGHT_TYPE_POINT:
		default:
			ASSERT(false);
			return glm::identity<glm::mat4>();
		}
	}

	void LightNode::ComputeVolumeInfo(const std::vector<MeshNode*>& meshNodes) noexcept
	{
		switch (type)
		{
		case LightType::LIGHT_TYPE_DIRECTIONAL:
			ComputeDirectionalVolumeInfo(meshNodes);
			break;

		case LightType::LIGHT_TYPE_POINT:
		default:
			ASSERT(false);
		}
	}

	void LightNode::ComputeDirectionalVolumeInfo(const std::vector<MeshNode*>& meshNodes) noexcept
	{
		glm::quat worldRot = GetWorldRotation();
		volumeInfo.directional.viewPos = glm::rotate(worldRot, glm::vec3(0.f, 0.f, 10.f));

		volumeInfo.directional.left = -20.f;
		volumeInfo.directional.right = 20.f;
		volumeInfo.directional.bottom = -20.f;
		volumeInfo.directional.top = 20.f;
		volumeInfo.directional.nearDist = 0.01f;
		volumeInfo.directional.farDist = 40.f;
	}

} // namespace lux::scene