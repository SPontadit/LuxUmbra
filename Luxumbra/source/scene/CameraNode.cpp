#include "scene\CameraNode.h"

namespace lux::scene
{
	CameraNode::CameraNode(Node* parent, const Window* window, float fovy, float nearDist, float farDist) noexcept
		: Node(parent), window(window), fovy(fovy), nearDist(nearDist), farDist(farDist)
	{

	}

	CameraNode::CameraNode(Node* parent, glm::vec3 position, glm::quat rotation, const Window* window, float fovy, float nearDist, float farDist) noexcept
		: Node(parent, position, rotation), window(window), fovy(fovy), nearDist(nearDist), farDist(farDist)
	{

	}

	glm::mat4 CameraNode::GetViewTransform() const noexcept
	{
		glm::vec3 eye = GetWorldPosition();
		glm::vec3 center = eye + glm::rotate(GetWorldRotation(), glm::vec3(0.f, 0.f, -1.f));
		glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);

		return glm::lookAtRH(eye, center, up);;
	}

	glm::mat4 CameraNode::GetPerspectiveProjectionTransform() const noexcept
	{
		float aspect = window->GetAspect();

		glm::mat4 perspectiveTransform = glm::perspective(glm::radians(fovy), aspect, nearDist, farDist);

		perspectiveTransform[1][1] *= -1.f;

		return perspectiveTransform;
	}
}