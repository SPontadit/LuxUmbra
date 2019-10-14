#ifndef MESH_NDOE_H_INCLUDED
#define MESH_NDOE_H_INCLUDED

#include "Luxumbra.h"

#include "scene\Node.h"
#include "resource\Mesh.h"

namespace lux::scene
{

	using namespace lux;

	class MeshNode : public Node
	{
	public:
		MeshNode() = delete;
		MeshNode(Node* parent, const std::shared_ptr<resource::Mesh>& mesh) noexcept;
		MeshNode(Node* parent, glm::vec3 position, glm::quat rotation, const std::shared_ptr<resource::Mesh>& mesh) noexcept;
		MeshNode(const MeshNode&) = delete;
		MeshNode(MeshNode&&) = delete;

		~MeshNode() noexcept = default;

		const MeshNode& operator=(const MeshNode&) = delete;
		const MeshNode& operator=(MeshNode&&) = delete;

	private:
		std::shared_ptr<resource::Mesh> mesh;
	};

} // namespace lux::scene

#endif // MESH_NDOE_H_INCLUDED