#include "Engine.h"

int main(int ac, char* av[])
{
	lux::Engine luxUmbra;

	luxUmbra.Initialize(1200, 800);

	lux::scene::Scene& scene = luxUmbra.GetScene();
	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();

	std::shared_ptr<lux::resource::Texture> texture = resourceManager.GetTexture("data/textures/Diffuse_Floor.jpg");

	resourceManager.UseCubemap("data/envmaps/quarry_02_2k.hdr");

	lux::resource::MaterialCreateInfo defaultMaterialCI;
	defaultMaterialCI.baseColor = glm::vec3(1.0f);
	defaultMaterialCI.metallic = false;
	defaultMaterialCI.perceptualRoughness = 0.5f;
	defaultMaterialCI.reflectance = 0.5f;
	defaultMaterialCI.albedo = texture;

	resourceManager.CreateMaterial("White", defaultMaterialCI);

	defaultMaterialCI.baseColor = glm::vec3(1.0f, 0.0f, 0.0f);
	resourceManager.CreateMaterial("Red", defaultMaterialCI);

	defaultMaterialCI.baseColor = glm::vec3(0.0f, 1.0f, 0.0f);
	resourceManager.CreateMaterial("Green", defaultMaterialCI);

	defaultMaterialCI.baseColor = glm::vec3(0.0f, 0.0f, 1.0f);
	resourceManager.CreateMaterial("Blue", defaultMaterialCI);


	scene.AddCameraNode(nullptr, { 2.5f, 5.f, 20.f }, glm::identity<glm::quat>(), false, 45.f, 0.01f, 1000.f, true);

	for (size_t i = 0; i < 5; i++)
	{
		scene.AddMeshNode(nullptr, { i * 3.0f - 2.0f, 5.f, 0.f }, glm::quat(glm::radians(glm::vec3( 90.0f / 5.0f * i, 0.0f, 0.0f))), false, "data/models/Sphere.fbx", "White");
		//scene.AddMeshNode(nullptr, { i * 3.0f - 2.0f, 10.f, 0.f }, glm::quat(glm::radians(glm::vec3( 90.0f / 5.0f * i, 0.0f, 0.0f))), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "White");
		scene.AddMeshNode(nullptr, { i * 3.0f - 2.0f, 10.f, 0.f }, glm::quat(glm::radians(glm::vec3(90.0f / 5.0f * i, 0.0f, 0.0f))), false, "data/models/PreviewMaterial.fbx", "White");
		//scene.AddMeshNode(nullptr, { i * 2.0f, 5.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Red");
		//scene.AddMeshNode(nullptr, { i * 2.0f, 15.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "White");
		//scene.AddMeshNode(nullptr, { i * 2.0f, 15.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Blue");
		//scene.AddMeshNode(nullptr, { i * 2.0f, 20.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Green");
	}

	//scene.AddMeshNode(nullptr, { 5.f, 0.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Default");
	scene.AddLightNode(nullptr, { 0.0f, 0.0f, -1.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 0.5f, 0.0f, 0.0f});
	scene.AddLightNode(nullptr, { 0.0f, 1.0f, 0.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 0.0f, 0.5f, 0.0f});
	scene.AddLightNode(nullptr, { 0.0f, 0.0f, 1.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 0.0f, 0.0f, 0.5f });

	luxUmbra.Run();

	return 0;
}