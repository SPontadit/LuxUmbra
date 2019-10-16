#include "scene\MeshNode.h"

namespace lux::scene
{

	using namespace lux;

	MeshNode::MeshNode(Node* parent, const std::shared_ptr<resource::Mesh>& mesh, const std::shared_ptr<resource::Material>& material) noexcept
		: Node(parent), mesh(mesh), material(material)
	{

	}

	MeshNode::MeshNode(Node* parent, glm::vec3 position, glm::quat rotation, const std::shared_ptr<resource::Mesh>& mesh, const std::shared_ptr<resource::Material>& material) noexcept
		: Node(parent, position, rotation), mesh(mesh), material(material)
	{

	}

	const resource::Mesh& MeshNode::GetMesh() const noexcept
	{
		return *mesh.get();
	}

	resource::Material& MeshNode::GetMaterial() const noexcept
	{
		return *material.get();
	}


}// namespace lux::scene