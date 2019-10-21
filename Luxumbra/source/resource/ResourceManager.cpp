#include "resource\ResourceManager.h"

#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb\stb_image.h"

#include "glm\gtc\type_ptr.hpp"


namespace lux::resource
{
	using namespace lux;

	using meshesIterator = std::unordered_map<std::string, std::shared_ptr<Mesh>>::iterator;
	using meshesConstIterator = std::unordered_map<std::string, std::shared_ptr<Mesh>>::const_iterator;

	using materialsIterator = std::unordered_map<std::string, std::shared_ptr<Material>>::iterator;
	using materialsConstIterator = std::unordered_map<std::string, std::shared_ptr<Material>>::const_iterator;

	using texturesIterator = std::unordered_map<std::string, std::shared_ptr<Texture>>::iterator;
	using texturesConstIterator = std::unordered_map<std::string, std::shared_ptr<Texture>>::const_iterator;


	ResourceManager::ResourceManager(rhi::RHI&  rhi) noexcept
		: rhi(rhi), cubemap(nullptr), irradiance(nullptr), defaultWhite(nullptr)
	{
	}

	ResourceManager::~ResourceManager() noexcept
	{
		ClearMeshes();
		ClearMaterials();
		ClearTextures();
	}

	void ResourceManager::Initialize() noexcept
	{
		//BuildPrimitiveMeshes();
		LoadPrimitiveMehes();

		defaultWhite = LoadTexture("data/textures/DefaultWhite.jpg", true);
		defaultNormalMap = LoadTexture("data/textures/DefaultNormalMap.jpg", true);
	}

	std::shared_ptr<Material> ResourceManager::GetMaterial(const std::string& materialName) noexcept
	{
		materialsConstIterator material = materials.find(materialName);

		if (material == materials.cend())
		{
			return nullptr;
		}

		return std::shared_ptr<Material>(material->second);
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

	std::shared_ptr<Texture> ResourceManager::GetTexture(const std::string& filename) noexcept
	{
		texturesConstIterator texture = textures.find(filename);

		if (texture == textures.cend())
		{
			return LoadTexture(filename);
		}

		return std::shared_ptr<Texture>(texture->second);
	}

	std::shared_ptr<Mesh> ResourceManager::GetMesh(MeshPrimitive meshPrimitive) noexcept
	{
		ASSERT(meshPrimitive != MeshPrimitive::MESH_PRIMITIVE_COUNT);

		return primitiveMeshes[TO_SIZE_T(meshPrimitive)];
	}

	void ResourceManager::GenerateSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint16_t horizSegments, uint16_t vertiSegments, float sphereScale)
	{
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

	void ResourceManager::LoadPrimitiveMehes() noexcept
	{
		primitiveMeshes[TO_UINT32_T(MeshPrimitive::MESH_SPHERE_PRIMITIVE)] = LoadMesh("data/models/Sphere.fbx", true);
		primitiveMeshes[TO_UINT32_T(MeshPrimitive::MESH_CUBE_PRIMITIVE)] = LoadMesh("data/models/Cube.fbx", true);

		rhi.SetCubeMesh(primitiveMeshes[TO_UINT32_T(MeshPrimitive::MESH_CUBE_PRIMITIVE)]);
	}

	std::shared_ptr<Material> ResourceManager::CreateMaterial(const std::string& name, MaterialCreateInfo materialCI) noexcept
	{
		if (materialCI.albedo == nullptr)
			materialCI.albedo = defaultWhite;

		if (materialCI.normal == nullptr)
			materialCI.normal = defaultNormalMap;

		std::shared_ptr<Material> material = std::make_shared<Material>(name, materialCI);

		rhi.CreateMaterial(*material);

		materials[name] = material;

		return material;
	}

	void ResourceManager::UseCubemap(const std::string& filenames) noexcept
	{
		cubemap = std::make_shared<Texture>();
		irradiance = std::make_shared<Texture>();

		// Load Cubemap
		float* textureData;
		int textureWidth, textureHeight, textureChannels;

		textureData = stbi_loadf(filenames.c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);

		uint64_t imageSize = textureWidth * textureHeight * 4 * sizeof(float);

		// Create source image
		rhi::ImageCreateInfo imageCI = {};
		imageCI.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		imageCI.width = TO_UINT32_T(textureWidth);
		imageCI.height = TO_UINT32_T(textureHeight);
		imageCI.arrayLayers = 1;
		imageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCI.subresourceRangeLayerCount = 1;
		imageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
		imageCI.imageData = textureData;
		imageCI.imageSize = imageSize;
		
		rhi::Image source;
		rhi.CreateImage(imageCI, source);


		// Create Cubemap Image
		imageCI.arrayLayers = 6;
		imageCI.subresourceRangeLayerCount = 6;
		imageCI.imageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
		imageCI.imageData = nullptr;
		imageCI.imageSize = 0;
		imageCI.width = 1024;
		imageCI.height = 1024;

		rhi.CreateImage(imageCI, cubemap->image);


		// Create Irradiance Image
		imageCI.width = 128;
		imageCI.height = 128;
		rhi.CreateImage(imageCI, irradiance->image);


		// Generate Cubemap
		rhi.GenerateCubemapFromHDR(source, cubemap->image);
		rhi.GenerateIrradianceFromCubemap(cubemap->image, irradiance->image);

		rhi.CreateEnvMapDescriptorSet(cubemap->image);

		rhi.DestroyImage(source);
		stbi_image_free(textureData);
	}

	std::shared_ptr<Mesh> ResourceManager::LoadMesh(const std::string& filename, bool isPrimitive) noexcept
	{
		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
		Assimp::Importer importer;

		unsigned int postProcessFlags = aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GlobalScale | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace;
		const aiScene* scene = importer.ReadFile(filename, postProcessFlags);

		if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
		{
			Logger::Log(LogLevel::LOG_LEVEL_WARNING, "Failed to load mesh:", filename);
			//TODO: handle mesh that is not valid
		}
		
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		
		const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			for (size_t j = 0; j < scene->mMeshes[i]->mNumVertices; j++)
			{

				Vertex vertex;

				vertex.position = glm::make_vec3(&scene->mMeshes[i]->mVertices[j].x);
				vertex.normal = glm::make_vec3(&scene->mMeshes[i]->mNormals[j].x);
				vertex.textureCoordinate = glm::make_vec2(&scene->mMeshes[i]->mTextureCoords[0][j].x);
			
				vertex.tangent = glm::make_vec3((scene->mMeshes[i]->HasTangentsAndBitangents()) ? (&scene->mMeshes[i]->mTangents[j].x) : &Zero3D.x);
				vertex.bitangent = glm::make_vec3((scene->mMeshes[i]->HasTangentsAndBitangents()) ? (&scene->mMeshes[i]->mBitangents[j].x) : &Zero3D.x);

				//vertex.position.y *= -1.0f;
				//vertex.normal.y *= -1.0f;

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

		if (isPrimitive == false)
			meshes[filename] = mesh;

		return mesh;
	}

	std::shared_ptr<Texture> ResourceManager::LoadTexture(const std::string& filename, bool isPrimitive) noexcept
	{
		std::shared_ptr<Texture> texture = std::make_shared<Texture>();

		int textureWidth, textureHeight, textureChannels;

		stbi_uc* textureData = stbi_load(filename.c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
	
		uint64_t imageSize = textureWidth * textureHeight * 4;

		rhi::ImageCreateInfo imageCI = {};
		imageCI.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageCI.width = TO_UINT32_T(textureWidth);
		imageCI.height = TO_UINT32_T(textureHeight);
		imageCI.arrayLayers = 1;
		imageCI.subresourceRangeAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCI.subresourceRangeLayerCount = 1;
		imageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCI.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
		imageCI.imageData = textureData;
		imageCI.imageSize = imageSize;

		rhi.CreateImage(imageCI, texture->image);

		stbi_image_free(textureData);
	
		if (isPrimitive == false)
			textures[filename] = texture;

		return texture;
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

	void ResourceManager::ClearMaterials() noexcept
	{
		materialsConstIterator it = materials.cbegin();
		materialsConstIterator itE = materials.cend();

		for (; it != itE; ++it)
		{
			rhi.DestroyMaterial((*it->second));
		}

		materials.clear();
	}

	void ResourceManager::ClearTextures() noexcept
	{
		texturesConstIterator it = textures.cbegin();
		texturesConstIterator itE = textures.cend();

		for (; it != itE; ++it)
		{
			rhi.DestroyImage(it->second->image);
		}

		textures.clear();

		rhi.DestroyImage(defaultWhite->image);
		rhi.DestroyImage(defaultNormalMap->image);

		if (cubemap != nullptr)
		{
			rhi.DestroyImage(cubemap->image);
		}

		if (irradiance != nullptr)
		{
			rhi.DestroyImage(irradiance->image);
		}
	}

} // namespace lux::resource