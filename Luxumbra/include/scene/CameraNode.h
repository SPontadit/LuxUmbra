#ifndef CAMERA_NODE_H_INCLUDED
#define CAMERA_NODE_H_INCLUDED

#include "Luxumbra.h"

#include "scene\Node.h"
#include "Window.h"

namespace lux::scene
{

	class CameraNode : public Node
	{
	public:
		CameraNode() = delete;
		CameraNode(Node* parent,  const Window* window, float fovy, float nearDist, float farDist) noexcept;
		CameraNode(Node* parent, glm::vec3 position, glm::vec3 rotation, const Window* window, float fovy, float nearDist, float farDist) noexcept;
		CameraNode(const CameraNode&) = delete;
		CameraNode(CameraNode&&) = delete;

		virtual ~CameraNode() noexcept = default;

		const CameraNode& operator=(const CameraNode&) = delete;
		const CameraNode& operator=(CameraNode&&) = delete;

		glm::mat4 GetViewTransform() const noexcept;
		glm::mat4 GetPerspectiveProjectionTransform() const noexcept;
		float GetNearDistance() const noexcept;
		float GetFarDistance() const noexcept;

		

	private:
		const Window* window;
		float fovy, nearDist, farDist;
	};

} // namespace lux::scene

#endif // CAMERA_NODE_H_INCLUDED