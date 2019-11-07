#include "scene\CameraNode.h"

namespace lux::scene
{
	CameraNode::CameraNode(Node* parent, const Window* window, float fovy, float nearDist, float farDist) noexcept
		: Node(parent), window(window), fovy(fovy), nearDist(nearDist), farDist(farDist)
	{

	}

	CameraNode::CameraNode(Node* parent, glm::vec3 position, glm::vec3 rotation, const Window* window, float fovy, float nearDist, float farDist) noexcept
		: Node(parent, position, rotation), window(window), fovy(fovy), nearDist(nearDist), farDist(farDist)
	{

	}

	glm::mat4 CameraNode::GetViewTransform() const noexcept
	{
		return glm::inverse(GetWorldTransform());
	}

	glm::mat4 CameraNode::GetPerspectiveProjectionTransform() const noexcept
	{
		float aspect = window->GetAspect();

		glm::mat4 perspectiveTransform = glm::perspective(glm::radians(fovy), aspect, nearDist, farDist);

		perspectiveTransform[1][1] *= -1.f;

		return perspectiveTransform;
	}

	float CameraNode::GetNearDistance() const noexcept
	{
		return nearDist;
	}

	float CameraNode::GetFarDistance() const noexcept
	{
		return farDist;
	}
}