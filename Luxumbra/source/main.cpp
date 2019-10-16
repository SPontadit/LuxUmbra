#include "Engine.h"

int main(int ac, char* av[])
{
	lux::Engine luxUmbra;

	luxUmbra.Initialize(1200, 800);

	lux::scene::Scene& scene = luxUmbra.GetScene();
	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();

	lux::resource::MaterialParameters defaultMaterialParameters;
	defaultMaterialParameters.baseColor = glm::vec3(1.0f);
	defaultMaterialParameters.metallic = false;
	defaultMaterialParameters.perceptualRoughness = 0.5f;
	defaultMaterialParameters.reflectance = 0.5f;

	resourceManager.CreateMaterial("White", defaultMaterialParameters);

	defaultMaterialParameters.baseColor = glm::vec3(1.0f, 0.0f, 0.0f);
	resourceManager.CreateMaterial("Red", defaultMaterialParameters);

	defaultMaterialParameters.baseColor = glm::vec3(0.0f, 1.0f, 0.0f);
	resourceManager.CreateMaterial("Green", defaultMaterialParameters);

	defaultMaterialParameters.baseColor = glm::vec3(0.0f, 0.0f, 1.0f);
	resourceManager.CreateMaterial("Blue", defaultMaterialParameters);


	scene.AddCameraNode(nullptr, { 2.5f, 5.f, 35.f }, glm::identity<glm::quat>(), false, 45.f, 0.01f, 1000.f, true);

	for (size_t i = 0; i < 5; i++)
	{
		scene.AddMeshNode(nullptr, { i * 2.0f, 0.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "White");
		scene.AddMeshNode(nullptr, { i * 2.0f, 5.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Red");
		scene.AddMeshNode(nullptr, { i * 2.0f, 10.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Green");
		scene.AddMeshNode(nullptr, { i * 2.0f, 15.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Blue");
	}
	//scene.AddMeshNode(nullptr, { 0.f, 0.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Default");
	//scene.AddMeshNode(nullptr, { 5.f, 0.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Default");
	scene.AddLightNode(nullptr, { 0.0f, 0.0f, -1.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 0.5f, 0.0f, 0.0f});
	scene.AddLightNode(nullptr, { 0.0f, 1.0f, 0.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 0.0f, 0.5f, 0.0f});
	scene.AddLightNode(nullptr, { 0.0f, 0.0f, 1.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 0.0f, 0.0f, 0.5f });
	scene.AddLightNode(nullptr, { 0.0f, 0.0f, 1.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 0.0f, 1.0f, 1.0f});

	luxUmbra.Run();

	return 0;
}