#include "Engine.h"

int main(int ac, char* av[])
{
	lux::Engine luxUmbra;

	luxUmbra.Initialize(1200, 800);

	lux::scene::Scene& scene = luxUmbra.GetScene();
	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();

	std::shared_ptr<lux::resource::Texture> albedo = resourceManager.GetTexture("data/textures/Brick_Diffuse.jpg");
	std::shared_ptr<lux::resource::Texture> mask = resourceManager.GetTexture("data/textures/Vulkan_Logo.png");
	std::shared_ptr<lux::resource::Texture> normal = resourceManager.GetTexture("data/textures/Brick_Normal.jpg");
	std::shared_ptr<lux::resource::Texture> box = resourceManager.GetTexture("data/textures/box.png");

	auto ironmanDif = resourceManager.GetTexture("data/textures/ironman.dff.png");
	auto ironmanNrm = resourceManager.GetTexture("data/textures/ironman.norm.png");

	resourceManager.UseCubemap("data/envmaps/Ridgecrest_Road_Ref.hdr");



	//defaultMaterialCI.normal = nullptr;
	//defaultMaterialCI.isTransparent = true;
	//defaultMaterialCI.albedo = mask;
	//resourceManager.CreateMaterial("Transparent", defaultMaterialCI);

	//defaultMaterialCI.albedo = ironmanDif;
	//defaultMaterialCI.isTransparent = false;
	//defaultMaterialCI.normal = ironmanNrm;
	//resourceManager.CreateMaterial("ironman", defaultMaterialCI);


	//scene.AddCameraNode(nullptr, { 2.5f, 5.f, 20.f }, { 0.f, 0.f, 0.f }, false, 45.f, 0.01f, 1000.f, true);
	scene.AddCameraNode(nullptr, {-2.0f, 5.0f, 20.0f }, { 0.f, 0.f, 0.f }, false, 45.f, 0.01f, 1000.f, true);


	lux::resource::MaterialCreateInfo defaultMaterialCI;
	defaultMaterialCI.baseColor = glm::vec3(1.0f);
	defaultMaterialCI.metallic = false;
	defaultMaterialCI.perceptualRoughness = 0.0f;
	defaultMaterialCI.reflectance = 0.0f;
	defaultMaterialCI.isTransparent = false;
	resourceManager.CreateMaterial("White", defaultMaterialCI);


	defaultMaterialCI.albedo = ironmanDif;
	defaultMaterialCI.isTransparent = false;
	defaultMaterialCI.normal = ironmanNrm;
	resourceManager.CreateMaterial("ironman", defaultMaterialCI);

	defaultMaterialCI.normal = nullptr;
	defaultMaterialCI.isTransparent = true;
	defaultMaterialCI.albedo = box;
	resourceManager.CreateMaterial("debug_box", defaultMaterialCI);

	scene.AddCameraNode(nullptr, { 0.f, 1.f, 5.f }, { 0.f, 0.f, 0.f }, false, 45.f, 0.01f, 1000.f, true);

	lux::scene::MeshNode* plane = scene.AddMeshNode(nullptr, glm::vec3(0.f, 1.f, -2.f), glm::radians(glm::vec3(0.f, 90.f, 0.f)), false, "data/models/Plane.fbx", "White");
	plane->SetLocalScale(glm::vec3(0.01f, 0.01f, 0.01f));

	plane = scene.AddMeshNode(nullptr, glm::vec3(-2.f, 1.f, 0.f), glm::radians(glm::vec3(0.f, 180.f, 0.f)), false, "data/models/Plane.fbx", "White");
	plane->SetLocalScale(glm::vec3(0.01f, 0.01f, 0.01f));

	plane = scene.AddMeshNode(nullptr, glm::vec3(2.f, 1.f, 0.f), glm::radians(glm::vec3(0.f, 0.f, 0.f)), false, "data/models/Plane.fbx", "White");
	plane->SetLocalScale(glm::vec3(0.01f, 0.01f, 0.01f));

	scene.AddMeshNode(nullptr, glm::vec3(0.f), glm::vec3(0.f), false, "data/models/ironman.fbx", "ironman");

	scene.AddLightNode(nullptr, { 0.0f, 0.0f, 0.0f }, glm::radians(glm::vec3(0.f, 15.f, 0.f)), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 1.0f, 1.0f, 1.0f });
	scene.AddLightNode(nullptr, { 0.0f, 0.0f, 0.0f }, glm::radians(glm::vec3(0.f, -15.f, 0.f)), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 1.0f, 1.0f, 1.0f });
	scene.AddLightNode(nullptr, { 0.f, 1.f, -1.5f }, { 0.f, 0.f, 0.f }, false, lux::scene::LightType::LIGHT_TYPE_POINT, glm::vec3(1.f, 0.f, 0.f));

	luxUmbra.Run();

	return 0;
}

//void TestSphere()
//{
//	lux::resource::MaterialCreateInfo defaultMaterialCI;
//	defaultMaterialCI.baseColor = glm::vec3(1.0f, 0.0f, 0.0f);
//	defaultMaterialCI.metallic = false;
//	defaultMaterialCI.perceptualRoughness = 0.0f;
//	defaultMaterialCI.reflectance = 0.0f;
//	defaultMaterialCI.isTransparent = false;
//
//	size_t row = 5;
//	size_t column = 10;
//
//	for (size_t i = 0; i < row; i++)
//	{
//		defaultMaterialCI.reflectance = TO_FLOAT(i) / TO_FLOAT(row);
//
//		for (size_t j = 0; j < column; j++)
//		{
//
//			defaultMaterialCI.perceptualRoughness = TO_FLOAT(j) / TO_FLOAT(column);
//
//			defaultMaterialCI.metallic = false;
//			std::string name("White_" + std::to_string(i) + "_" + std::to_string(j));
//			resourceManager.CreateMaterial(name, defaultMaterialCI);
//			scene.AddMeshNode(nullptr, { j, i, 0.f }, { 0.0f, 0.0f, 0.0f }, false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, name);
//		}
//	}
//
//	for (size_t j = 0; j < column; j++)
//	{
//		defaultMaterialCI.perceptualRoughness = TO_FLOAT(j) / TO_FLOAT(column);
//
//		defaultMaterialCI.metallic = true;
//		std::string name = ("Metallic_" + std::to_string(j));
//		resourceManager.CreateMaterial(name, defaultMaterialCI);
//		scene.AddMeshNode(nullptr, { j, row, 0.f }, { 0.0f, 0.0f, 0.0f }, false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, name);
//
//	}
//
//	//  scene.AddMeshNode(nullptr, { i * 3.0f - 2.0f, 10.f, 0.f }, glm::radians(glm::vec3(90.0f / 5.0f * i, 0.0f, 0.0f)), false, lux::resource::MeshPrimitive::MESH_CUBE_PRIMITIVE, "Transparent");
//	//	scene.AddMeshNode(nullptr, { i * 3.0f - 2.0f, 0.f, 0.f }, glm::radians(glm::vec3(90.0f / 5.0f * i, 0.0f, 0.0f)), false, "data/models/ironman.fbx", "ironman");
//	scene.AddLightNode(nullptr, { 0.0f, 0.0f, -1.0f }, { 0.f, 0.f, 0.f }, false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 1.0f, 1.0f, 1.0f });
//	scene.AddLightNode(nullptr, { 0.0f, 1.0f, -1.0f }, { 0.f, 0.f, 0.f }, false, lux::scene::LightType::LIGHT_TYPE_POINT, { 0.0f, 0.0f, 0.0f });
//	scene.AddLightNode(nullptr, { 0.0f, 0.0f, 1.0f }, { 0.f, 0.f, 0.f }, false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 0.0f, 0.0f, 0.0f });
//}