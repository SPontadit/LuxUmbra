#ifndef MESH_NDOE_H_INCLUDED
#define MESH_NDOE_H_INCLUDED

#include "Luxumbra.h"

#include "scene\Node.h"
#include "resource\Mesh.h"
#include "resource\Material.h"

namespace lux::scene
{

	using namespace lux;

	class MeshNode : public Node
	{
	public:
		MeshNode() = delete;
		MeshNode(Node* parent, const std::shared_ptr<resource::Mesh>& mesh, const std::shared_ptr<resource::Material>& material) noexcept;
		MeshNode(Node* parent, glm::vec3 position, glm::vec3 rotation, const std::shared_ptr<resource::Mesh>& mesh, const std::shared_ptr<resource::Material>& material) noexcept;
		MeshNode(const MeshNode&) = delete;
		MeshNode(MeshNode&&) = delete;

		~MeshNode() noexcept = default;

		const MeshNode& operator=(const MeshNode&) = delete;
		const MeshNode& operator=(MeshNode&&) = delete;

		const resource::Mesh& GetMesh() const noexcept;
		resource::Material& GetMaterial() const noexcept;

	private:
		std::shared_ptr<resource::Mesh> mesh;
		std::shared_ptr<resource::Material> material;
	};

} // namespace lux::scene

#endif // MESH_NDOE_H_INCLUDED