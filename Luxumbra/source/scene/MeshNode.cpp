#include "scene\MeshNode.h"

namespace lux::scene
{

	using namespace lux;

	MeshNode::MeshNode(Node* parent, const std::shared_ptr<resource::Mesh>& mesh) noexcept
		: Node(parent), mesh(mesh)
	{

	}

	MeshNode::MeshNode(Node* parent, glm::vec3 position, glm::quat rotation, const std::shared_ptr<resource::Mesh>& mesh) noexcept
		: Node(parent, position, rotation), mesh(mesh)
	{

	}

	const resource::Mesh& MeshNode::GetMesh() const noexcept
	{
		return *mesh.get();
	}

}// namespace lux::scene