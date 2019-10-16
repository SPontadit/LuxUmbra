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

	resourceManager.CreateMaterial("Default", defaultMaterialParameters);

	scene.AddCameraNode(nullptr, { 0.f, 0.f, 5.f }, glm::identity<glm::quat>(), false, 45.f, 0.01f, 1000.f, true);
	scene.AddMeshNode(nullptr, { 0.f, 0.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Default");
	scene.AddMeshNode(nullptr, { 5.f, 0.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Default");
	scene.AddLightNode(nullptr, { 0.0f, 0.0f, -1.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 0.5f, 0.0f, 0.0f});
	scene.AddLightNode(nullptr, { 0.0f, 1.0f, 0.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 0.0f, 0.5f, 0.0f});
	scene.AddLightNode(nullptr, { 0.0f, 0.0f, 1.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 0.0f, 0.0f, 0.5f });
	scene.AddLightNode(nullptr, { 0.0f, 0.0f, 1.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 0.0f, 1.0f, 1.0f});

	luxUmbra.Run();

	return 0;
}