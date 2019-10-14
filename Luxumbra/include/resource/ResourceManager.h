#ifndef RESOURCE_MANAGER_H_INCLUDED
#define RESOURCE_MANAGER_H_INCLUDED

#include "Luxumbra.h"

#include <unordered_map>
#include <array>

#include "rhi\RHI.h"
#include "resource\Mesh.h"
#include "Vertex.h"

namespace lux::resource
{
	using namespace lux;

	enum class MeshPrimitive
	{
		MESH_SPHERE_PRIMITIVE = 0,
		MESH_PRIMITIVE_COUNT
	};

	class ResourceManager
	{
	public:
		ResourceManager(rhi::RHI& rhi) noexcept;
		ResourceManager(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&&) = delete;

		~ResourceManager() noexcept;

		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager& operator=(ResourceManager&&) = delete;

		void Initialize() noexcept;
	
		std::shared_ptr<Mesh> GetMesh(const std::string& filename) noexcept;
		std::shared_ptr<Mesh> GetMesh(MeshPrimitive meshPrimitive) noexcept;

	private:
		void BuildPrimitiveMeshes() noexcept;
		void GenerateSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint16_t horizSegments, uint16_t vertiSegments, float sphereScale = 1.f);

		std::shared_ptr<Mesh> LoadMesh(const std::string& filename) noexcept;

		rhi::RHI& rhi;

		std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
		std::array<std::shared_ptr<Mesh>, TO_SIZE_T(MeshPrimitive::MESH_PRIMITIVE_COUNT)> primitiveMeshes;
	};

} // namespace lux::resource

#endif // RESOURCE_MANAGER_H_INCLUDED