#include "Engine.h"

int main(int ac, char* av[])
{
	lux::Engine luxUmbra;

	luxUmbra.Initialize(1200, 800);

	lux::scene::Scene& scene = luxUmbra.GetScene();

	scene.AddCameraNode(nullptr, { 0.f, 0.f, 20.f }, glm::identity<glm::quat>(), false, 45.f, 0.01f, 1000.f, true);
	scene.AddMeshNode(nullptr, { 0.f, 0.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE);
	scene.AddMeshNode(nullptr, { 5.f, 0.f, 0.f }, glm::identity<glm::quat>(), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE);
	scene.AddLightNode(nullptr, { 1.0f, 0.0f, 0.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL);
	scene.AddLightNode(nullptr, { 0.0f, 1.0f, 0.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL);
	scene.AddLightNode(nullptr, { 0.0f, 0.0f, 1.0f }, glm::identity<glm::quat>(), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL);

	luxUmbra.Run();

	return 0;
}