#include "resource\ResourceManager.h"

#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"
#include "glm\gtc\type_ptr.hpp"


namespace lux::resource
{
	using namespace lux;

	using meshesIterator = std::unordered_map<std::string, std::shared_ptr<Mesh>>::iterator;
	using meshesConstIterator = std::unordered_map<std::string, std::shared_ptr<Mesh>>::const_iterator;

	ResourceManager::ResourceManager(rhi::RHI&  rhi) noexcept
		: rhi(rhi)
	{
	}

	ResourceManager::~ResourceManager() noexcept
	{
		ClearMeshes();
	}

	void ResourceManager::Initialize() noexcept
	{
		BuildPrimitiveMeshes();
	}

	std::shared_ptr<Mesh> ResourceManager::GetMesh(const std::string& filename) noexcept
	{
		meshesConstIterator mesh = meshes.find(filename);

		if (mesh == meshes.cend())
		{
			return LoadMesh(filename);
		}

		return std::shared_ptr<Mesh>(mesh->second);
	}

	std::shared_ptr<Mesh> ResourceManager::GetMesh(MeshPrimitive meshPrimitive) noexcept
	{
		ASSERT(meshPrimitive != MeshPrimitive::MESH_PRIMITIVE_COUNT);

		return primitiveMeshes[TO_SIZE_T(meshPrimitive)];
	}

	void ResourceManager::GenerateSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint16_t horizSegments, uint16_t vertiSegments, float sphereScale)
	{
		const float PI = 3.14159265359f;

		indices.reserve((vertiSegments + 1)*(horizSegments + 1));
		vertices.reserve((vertiSegments + 1)*(horizSegments + 1));

		for (uint16_t y = 0; y <= vertiSegments; ++y)
		{
			for (uint16_t x = 0; x <= horizSegments; ++x)
			{
				float xSegment = TO_FLOAT(x) / TO_FLOAT(horizSegments);
				float ySegment = TO_FLOAT(y) / TO_FLOAT(vertiSegments);
				float theta = ySegment * PI;
				float phi = xSegment * 2.0f * PI;
				float xPos = std::cos(phi) * std::sin(theta);
				float yPos = std::cos(theta);
				float zPos = std::sin(phi) * std::sin(theta);
				Vertex vertex{
					glm::vec3(xPos*sphereScale, yPos*sphereScale, zPos*sphereScale),
					glm::vec2(xSegment, ySegment),
					glm::vec3(xPos, yPos, zPos)
				};
				vertices.push_back(vertex);
			}
		}

		bool oddRow = false;
		for (int y = 0; y < vertiSegments; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (int x = 0; x <= horizSegments; ++x)
				{
					// (y) suivi de (y+1) -> CW
					// (y+1) suivi de (y) -> CCW
					indices.push_back((y + 1) * (horizSegments + 1) + x);
					indices.push_back(y       * (horizSegments + 1) + x);
				}
			}
			else
			{
				for (int x = horizSegments; x >= 0; --x)
				{
					// (y+1) suivi de (y) -> CW
					// (y) suivi de (y+1) -> CCW
					indices.push_back(y       * (horizSegments + 1) + x);
					indices.push_back((y + 1) * (horizSegments + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
	}

	void ResourceManager::BuildPrimitiveMeshes() noexcept
	{
		std::vector<Vertex> sphereVertices;
		std::vector<uint32_t> sphereIndices;

		GenerateSphere(sphereVertices, sphereIndices, 64, 64);
		
		// TODO: use staging buffer
		rhi::BufferCreateInfo vertexBufferCI = {};
		vertexBufferCI.usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vertexBufferCI.size = sizeof(Vertex) * sphereVertices.size();
		vertexBufferCI.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		vertexBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vertexBufferCI.data = sphereVertices.data();

		rhi::BufferCreateInfo indexBufferCI = {};
		indexBufferCI.usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indexBufferCI.size = sizeof(uint32_t) * sphereIndices.size();
		indexBufferCI.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		indexBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		indexBufferCI.data = sphereIndices.data();

		std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>();

		rhi.CreateBuffer(vertexBufferCI, sphereMesh->vertexBuffer);
		rhi.CreateBuffer(indexBufferCI, sphereMesh->indexBuffer);
	
		sphereMesh->indexCount = TO_UINT32_T(sphereIndices.size());

		primitiveMeshes[TO_SIZE_T(MeshPrimitive::MESH_SPHERE_PRIMITIVE)] = sphereMesh;
	}

	std::shared_ptr<Mesh> ResourceManager::LoadMesh(const std::string& filename) noexcept
	{
		std::shared_ptr<Mesh> mesh = std::make_shared <Mesh>();
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Fast);

		if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
		{
			Logger::Log(LogLevel::LOG_LEVEL_WARNING, "Failed to load mesh:", filename);
			//TODO: handle mesh that is not valid
		}
		
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		
		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			for (size_t j = 0; j < scene->mMeshes[i]->mNumVertices; j++)
			{
				Vertex vertex;

				vertex.position = glm::make_vec3(&scene->mMeshes[i]->mVertices[j].x);
				vertex.normal = glm::make_vec3(&scene->mMeshes[i]->mNormals[j].x);
				vertex.textureCoordinate = glm::make_vec2(&scene->mMeshes[i]->mTextureCoords[0][j].x);
			
				vertex.position.y *= -1.0f;

				vertices.push_back(vertex);
			}
		}

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			uint32_t indexBase = TO_UINT32_T(indices.size());
			for (size_t j = 0; j < scene->mMeshes[i]->mNumFaces; j++)
			{
				for (size_t k = 0; k < 3; k++)
				{
					indices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[k] + indexBase);
				}
			}
		}
		
		rhi::BufferCreateInfo vertexBufferCI = {};
		vertexBufferCI.usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vertexBufferCI.size = sizeof(Vertex) * vertices.size();
		vertexBufferCI.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		vertexBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vertexBufferCI.data = vertices.data();

		rhi::BufferCreateInfo indexBufferCI = {};
		indexBufferCI.usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indexBufferCI.size = sizeof(uint32_t) * indices.size();
		indexBufferCI.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		indexBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		indexBufferCI.data = indices.data();

		rhi.CreateBuffer(vertexBufferCI, mesh->vertexBuffer);
		rhi.CreateBuffer(indexBufferCI, mesh->indexBuffer);

		mesh->indexCount = TO_UINT32_T(indices.size());

		return mesh;
	}

	void ResourceManager::ClearMeshes() noexcept
	{
		meshesConstIterator it = meshes.cbegin();
		meshesConstIterator itE = meshes.cend();

		for (; it != itE; ++it)
		{
			rhi.DestroyBuffer(it->second->vertexBuffer);
			rhi.DestroyBuffer(it->second->indexBuffer);
		}

		meshes.clear();

		for (size_t i = 0; i < primitiveMeshes.size(); i++)
		{
			rhi.DestroyBuffer(primitiveMeshes[i]->vertexBuffer);
			rhi.DestroyBuffer(primitiveMeshes[i]->indexBuffer);
		}
	}

} // namespace lux::resource