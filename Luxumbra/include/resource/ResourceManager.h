#ifndef RESOURCE_MANAGER_H_INCLUDED
#define RESOURCE_MANAGER_H_INCLUDED

#include "Luxumbra.h"

#include <unordered_map>
#include <array>

#include "rhi\RHI.h"
#include "resource\Mesh.h"
#include "resource\Texture.h"
#include "resource\Material.h"
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
	
		std::shared_ptr<Material> CreateMaterial(const std::string& name, MaterialCreateInfo materialCI) noexcept;

		std::shared_ptr<Material> GetMaterial(const std::string& name) noexcept;
		std::shared_ptr<Mesh> GetMesh(const std::string& filename) noexcept;
		std::shared_ptr<Mesh> GetMesh(MeshPrimitive meshPrimitive) noexcept;
		std::shared_ptr<Texture> GetTexture(const std::string& filename) noexcept;

	private:
		void ClearMeshes() noexcept;
		void ClearMaterials() noexcept;
		void ClearTextures() noexcept;
		

		void BuildPrimitiveMeshes() noexcept;
		void GenerateSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint16_t horizSegments, uint16_t vertiSegments, float sphereScale = 1.f);

		std::shared_ptr<Mesh> LoadMesh(const std::string& filename) noexcept;
		std::shared_ptr<Texture> LoadTexture(const std::string& filename) noexcept;

		rhi::RHI& rhi;
		
		std::unordered_map<std::string, std::shared_ptr<Material>> materials;
		std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
		std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
		std::array<std::shared_ptr<Mesh>, TO_SIZE_T(MeshPrimitive::MESH_PRIMITIVE_COUNT)> primitiveMeshes;
	};

} // namespace lux::resource

#endif // RESOURCE_MANAGER_H_INCLUDED