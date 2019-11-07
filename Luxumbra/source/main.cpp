#include "Engine.h"

void BuildPostProcessScene(lux::Engine& luxUmbra) noexcept;
void BuildShadowScene(lux::Engine& luxUmbra) noexcept;
void BuildPBRModels(lux::Engine& luxUmbra) noexcept;
void BuildPBRMaterials(lux::Engine& luxUmbra) noexcept;
void BuildSphereScene(lux::Engine& luxUmbra) noexcept;
void CreateMaterials(lux::Engine& luxUmbra) noexcept;

int main(int ac, char* av[])
{
	lux::Engine luxUmbra;

	luxUmbra.Initialize(1200, 800);

	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();
	resourceManager.UseCubemap("data/envmaps/Ridgecrest_Road_Ref.hdr");

	//BuildSphereScene(luxUmbra);
	BuildPostProcessScene(luxUmbra);
	//BuildShadowScene(luxUmbra);
	//BuildPBRModels(luxUmbra);
	//BuildPBRMaterials(luxUmbra);

	luxUmbra.Run();

	return 0;
}

void BuildSphereScene(lux::Engine& luxUmbra) noexcept
{
	lux::scene::Scene& scene = luxUmbra.GetScene(lux::SCENE::SPHERE_SCENE);
	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();

	lux::resource::MaterialCreateInfo defaultMaterialCI;
	defaultMaterialCI.baseColor = glm::vec3(1.0f, 0.0f, 0.0f);
	defaultMaterialCI.metallic = false;
	defaultMaterialCI.perceptualRoughness = 0.0f;
	defaultMaterialCI.reflectance = 0.0f;
	defaultMaterialCI.isTransparent = false;

	size_t row = 3;
	size_t column = 7;

	for (size_t i = 0; i <= row; i++)
	{
		defaultMaterialCI.reflectance = TO_FLOAT(i) / TO_FLOAT(row);

		for (size_t j = 0; j <= column; j++)
		{

			defaultMaterialCI.perceptualRoughness = TO_FLOAT(j) / TO_FLOAT(column);

			defaultMaterialCI.metallic = false;
			std::string name("White_" + std::to_string(i) + "_" + std::to_string(j));
			resourceManager.CreateMaterial(name, defaultMaterialCI);
			scene.AddMeshNode(nullptr, { j, i, 0.f }, { 0.0f, 0.0f, 0.0f }, false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, name);
		}
	}

	defaultMaterialCI.baseColor = glm::vec3(1.0f, 1.0f, 1.0f);

	for (size_t j = 0; j < column; j++)
	{
		defaultMaterialCI.perceptualRoughness = TO_FLOAT(j) / TO_FLOAT(column);

		defaultMaterialCI.metallic = true;
		std::string name = ("Metallic_" + std::to_string(j));
		resourceManager.CreateMaterial(name, defaultMaterialCI);
		scene.AddMeshNode(nullptr, { j, row, 0.f }, { 0.0f, 0.0f, 0.0f }, false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, name);
	}

	defaultMaterialCI.baseColor = glm::vec3(1.0f, 0.0f, 0.0f);

	for (size_t j = 0; j < column; j++)
	{
		defaultMaterialCI.clearCoatPerceptualRoughness = 0.4f;
		defaultMaterialCI.clearCoat = TO_FLOAT(j) / TO_FLOAT(column);

		defaultMaterialCI.metallic = true;
		std::string name = ("ClearCoat_" + std::to_string(j));
		resourceManager.CreateMaterial(name, defaultMaterialCI);
		scene.AddMeshNode(nullptr, { j, row + 1.0f, 0.f }, { 0.0f, 0.0f, 0.0f }, false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, name);
	}

	for (size_t j = 0; j < column; j++)
	{
		defaultMaterialCI.clearCoat = 0.8f;
		defaultMaterialCI.clearCoatPerceptualRoughness = TO_FLOAT(j) / TO_FLOAT(column);

		defaultMaterialCI.metallic = false;
		std::string name = ("ClearCoatRough_" + std::to_string(j));
		resourceManager.CreateMaterial(name, defaultMaterialCI);
		scene.AddMeshNode(nullptr, { j, row + 2.0f, 0.f }, { 0.0f, 0.0f, 0.0f }, false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, name);
	}

	scene.AddLightNode(nullptr, { 0.0f, 0.0f, -1.0f }, glm::radians(glm::vec3(-45.f, 70.f, 0.f)), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 1.0f, 1.0f, 1.0f });

	scene.AddCameraNode(nullptr, { 3.f, 2.5f, 7.5f }, { 0.f, 0.f, 0.f }, false, 45.f, 0.01f, 1000.f, true);
}

void BuildShadowScene(lux::Engine& luxUmbra) noexcept
{
	lux::scene::Scene& scene = luxUmbra.GetScene(lux::SCENE::SHADOW_SCENE);

	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();

	std::shared_ptr<lux::resource::Texture> box = resourceManager.GetTexture("data/textures/box.png");

	std::shared_ptr<lux::resource::Texture> ironmanDif = resourceManager.GetTexture("data/textures/ironman.dff.png");
	std::shared_ptr<lux::resource::Texture> ironmanNrm = resourceManager.GetTexture("data/textures/ironman.norm.png");

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

	lux::scene::MeshNode* plane = scene.AddMeshNode(nullptr, glm::vec3(0.f, 1.f, -2.f), glm::radians(glm::vec3(0.f, 90.f, 0.f)), false, lux::resource::MeshPrimitive::MESH_PLANE_PRIMITIVE, "White");
	plane = scene.AddMeshNode(nullptr, glm::vec3(-2.f, 1.f, 0.f), glm::radians(glm::vec3(0.f, 180.f, 0.f)), false, lux::resource::MeshPrimitive::MESH_PLANE_PRIMITIVE, "White");
	plane = scene.AddMeshNode(nullptr, glm::vec3(2.f, 1.f, 0.f), glm::radians(glm::vec3(0.f, 0.f, 0.f)), false, lux::resource::MeshPrimitive::MESH_PLANE_PRIMITIVE, "White");

	scene.AddMeshNode(nullptr, glm::vec3(0.f), glm::vec3(0.f), false, "data/models/ironman.fbx", "ironman");

	scene.AddLightNode(nullptr, { 0.0f, 0.0f, -1.0f }, { 0.f, 0.f, 0.f }, false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 1.0f, 1.0f, 1.0f });
}

void BuildPBRModels(lux::Engine& luxUmbra) noexcept
{
	lux::scene::Scene& scene = luxUmbra.GetScene(lux::SCENE::PBR_MODELS_SCENE);
	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();

	std::shared_ptr<lux::resource::Texture> baseColorHelmet = resourceManager.GetTexture("data/DamagedHelmet/Default_albedo.jpg");
	std::shared_ptr<lux::resource::Texture> normalHelmet = resourceManager.GetTexture("data/DamagedHelmet/Default_normal.jpg");
	std::shared_ptr<lux::resource::Texture> metRoughHelmet = resourceManager.GetTexture("data/DamagedHelmet/Default_metalRoughness.jpg");
	std::shared_ptr<lux::resource::Texture> aoHelmet = resourceManager.GetTexture("data/DamagedHelmet/Default_AO.jpg");
	
	std::shared_ptr<lux::resource::Texture> baseColorScifiHelmet = resourceManager.GetTexture("data/SciFiHelmet/SciFiHelmet_BaseColor.png");
	std::shared_ptr<lux::resource::Texture> normalScifiHelmet = resourceManager.GetTexture("data/SciFiHelmet/SciFiHelmet_Normal.png");
	std::shared_ptr<lux::resource::Texture> metRoughScifiHelmet = resourceManager.GetTexture("data/SciFiHelmet/SciFiHelmet_MetallicRoughness.png");
	std::shared_ptr<lux::resource::Texture> aoScifiHelmet = resourceManager.GetTexture("data/SciFiHelmet/SciFiHelmet_AmbientOcclusion.png");

	std::shared_ptr<lux::resource::Texture> baseColorCorset = resourceManager.GetTexture("data/Corset/Corset_baseColor.png");
	std::shared_ptr<lux::resource::Texture> normalCorset = resourceManager.GetTexture("data/Corset/Corset_normal.png");
	std::shared_ptr<lux::resource::Texture> metRoughCorset = resourceManager.GetTexture("data/Corset/Corset_RoughnessMetallic.png");
	std::shared_ptr<lux::resource::Texture> aoCorset = resourceManager.GetTexture("data/Corset/Corset_AO.png");


	lux::resource::MaterialCreateInfo defaultMaterialCI;
	defaultMaterialCI.baseColor = glm::vec3(1.0f);
	defaultMaterialCI.perceptualRoughness = 0.0f;
	defaultMaterialCI.reflectance = 0.5f;
	defaultMaterialCI.isTransparent = false;
	defaultMaterialCI.metallic = 0.0f;

	defaultMaterialCI.albedo = baseColorHelmet;
	defaultMaterialCI.normal = normalHelmet;
	defaultMaterialCI.metallicRoughness = metRoughHelmet;
	defaultMaterialCI.textureMask = lux::resource::TextureMask::METALLIC_TEXTURE_MASK | lux::resource::TextureMask::ROUGHNESS_TEXTURE_MASK;
	defaultMaterialCI.ambientOcclusion = aoHelmet;
	resourceManager.CreateMaterial("Helmet", defaultMaterialCI);

	defaultMaterialCI.albedo = baseColorScifiHelmet;
	defaultMaterialCI.normal = normalScifiHelmet;
	defaultMaterialCI.metallicRoughness = metRoughScifiHelmet;
	defaultMaterialCI.ambientOcclusion = aoScifiHelmet;
	resourceManager.CreateMaterial("SciFiHelmet", defaultMaterialCI);

	defaultMaterialCI.albedo = baseColorCorset;
	defaultMaterialCI.normal = normalCorset;
	defaultMaterialCI.metallicRoughness = metRoughCorset;
	defaultMaterialCI.ambientOcclusion = aoCorset;
	defaultMaterialCI.reflectance = 0.2f;
	resourceManager.CreateMaterial("Corset", defaultMaterialCI);

	scene.AddCameraNode(nullptr, { 0.f, 1.f, 5.f }, { 0.f, 0.f, 0.f }, false, 45.f, 0.1f, 50.f, true);

	lux::scene::MeshNode* DamagedHelmet = scene.AddMeshNode(nullptr, glm::vec3(-2.0f, 0.5f, 0.f), glm::vec3(0.f), false, "data/DamagedHelmet/DamagedHelmet.gltf", "Helmet");
	DamagedHelmet->SetLocalScale(glm::vec3(1.0f));

	lux::scene::MeshNode* sciFiHelmet = scene.AddMeshNode(nullptr, glm::vec3(2.0f, 0.5f, 0.f), glm::vec3(0.f), false, "data/SciFiHelmet/SciFiHelmet.gltf", "SciFiHelmet");
	sciFiHelmet->SetLocalScale(glm::vec3(0.9f));

	lux::scene::MeshNode* corset = scene.AddMeshNode(nullptr, glm::vec3(0.0f, -0.5f, 0.f), glm::vec3(0.f), false, "data/Corset/Corset.gltf", "Corset");
	corset->SetLocalScale(glm::vec3(45.0f));

	lux::scene::LightNode* directionalLight = scene.AddLightNode(nullptr, glm::vec3(1.5f, 0.f, 0.f), { 0.f, 0.f, 0.f }, false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, glm::vec3(1.f, 1.f, 1.f));
}

void BuildPBRMaterials(lux::Engine& luxUmbra) noexcept
{
	lux::scene::Scene& scene = luxUmbra.GetScene(lux::SCENE::PBR_MATERIALS_SCENE);
	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();

	std::shared_ptr<lux::resource::Texture> albedoBloody = resourceManager.GetTexture("data/textures/BloodyGuts_basecolor.png");
	std::shared_ptr<lux::resource::Texture> normalBloody = resourceManager.GetTexture("data/textures/BloodyGuts_normal.png");
	std::shared_ptr<lux::resource::Texture> metRoughBloody = resourceManager.GetTexture("data/textures/BloodyGuts_roughness.png");
	std::shared_ptr<lux::resource::Texture> aoBloody = resourceManager.GetTexture("data/textures/BloodyGuts_AO.png");

	std::shared_ptr<lux::resource::Texture> albedoBlueGranite = resourceManager.GetTexture("data/textures/Blue_Granite_BaseColor.png");
	std::shared_ptr<lux::resource::Texture> metRoughBlueGranite = resourceManager.GetTexture("data/textures/Blue_Granite_Metallic_Roughness.png");

	std::shared_ptr<lux::resource::Texture> albedoMarble = resourceManager.GetTexture("data/textures/Marble_BaseColor.png");
	std::shared_ptr<lux::resource::Texture> normalMarble = resourceManager.GetTexture("data/textures/Marble_Normal.png");
	std::shared_ptr<lux::resource::Texture> roughMarble = resourceManager.GetTexture("data/textures/Marble_Roughness.png");


	lux::resource::MaterialCreateInfo defaultMaterialCI;
	defaultMaterialCI.baseColor = glm::vec3(1.0f);
	defaultMaterialCI.albedo = albedoBloody;
	defaultMaterialCI.metallicRoughness = metRoughBloody;
	defaultMaterialCI.perceptualRoughness = 0.0f;
	defaultMaterialCI.reflectance = 0.5f;
	defaultMaterialCI.isTransparent = false;
	defaultMaterialCI.metallic = 0.0f;
	defaultMaterialCI.textureMask = lux::resource::TextureMask::ROUGHNESS_TEXTURE_MASK;
	resourceManager.CreateMaterial("BloodyGuts", defaultMaterialCI);


	defaultMaterialCI.albedo = albedoBlueGranite;
	defaultMaterialCI.metallicRoughness = metRoughBlueGranite;
	defaultMaterialCI.textureMask = lux::resource::TextureMask::ROUGHNESS_TEXTURE_MASK | lux::resource::TextureMask::METALLIC_TEXTURE_MASK;
	defaultMaterialCI.reflectance = 0.0f;
	defaultMaterialCI.clearCoat = 1.0f;
	defaultMaterialCI.clearCoatPerceptualRoughness = 1.0;
	resourceManager.CreateMaterial("Blue_Granite", defaultMaterialCI);


	defaultMaterialCI.albedo = albedoMarble;
	defaultMaterialCI.normal = normalMarble;
	defaultMaterialCI.metallicRoughness = roughMarble;
	defaultMaterialCI.textureMask = lux::resource::TextureMask::ROUGHNESS_TEXTURE_MASK ;
	defaultMaterialCI.reflectance = 0.85f;
	defaultMaterialCI.clearCoat = 0.0f;
	defaultMaterialCI.clearCoatPerceptualRoughness = 0.0;
	resourceManager.CreateMaterial("Marble", defaultMaterialCI);

	std::shared_ptr<lux::resource::Texture> debugBox = resourceManager.GetTexture("data/textures/box.png");
	lux::resource::MaterialCreateInfo debugMaterialCI = {};
	debugMaterialCI.isTransparent = true;
	debugMaterialCI.albedo = debugBox;
	resourceManager.CreateMaterial("Debug", debugMaterialCI);

	scene.AddCameraNode(nullptr, { 0.f, 1.f, 5.f }, { 0.f, 0.f, 0.f }, false, 45.f, 0.1f, 50.f, true);

	scene.AddMeshNode(nullptr, glm::vec3(0.0f, 0.f, 0.f), glm::radians(glm::vec3(90.0f, 0.0f, 0.0f)), false, lux::resource::MeshPrimitive::MESH_CUBE_PRIMITIVE, "Debug");
	scene.AddMeshNode(nullptr, glm::vec3(-1.5f, 0.f, 0.f), glm::radians(glm::vec3(-90.0f, 0.0f, 0.0f)), false, lux::resource::MeshPrimitive::MESH_PREVIEW_MATERIAL_PRIMITIVE, "BloodyGuts");
	scene.AddMeshNode(nullptr, glm::vec3(0.0f, 0.f, 0.f), glm::radians(glm::vec3(-90.0f, 0.0f, 0.0f)), false, lux::resource::MeshPrimitive::MESH_PREVIEW_MATERIAL_PRIMITIVE, "Blue_Granite");
	scene.AddMeshNode(nullptr, glm::vec3(0.0f, 1.5f, 0.f), glm::radians(glm::vec3(-90.0f, 0.0f, 0.0f)), false, lux::resource::MeshPrimitive::MESH_PREVIEW_MATERIAL_PRIMITIVE, "Marble");

	lux::scene::LightNode* directionalLight = scene.AddLightNode(nullptr, glm::vec3(1.5f, 0.f, 0.f), { 0.f, 0.f, 0.f }, false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, glm::vec3(1.f, 1.f, 1.f));
}

void BuildPostProcessScene(lux::Engine& luxUmbra) noexcept
{
	lux::scene::Scene& scene = luxUmbra.GetScene(lux::SCENE::POST_PROCESS_SCENE);
	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();

	lux::resource::MaterialCreateInfo defaultMaterialCI;
	defaultMaterialCI.baseColor = glm::vec3(1.0f);
	defaultMaterialCI.metallic = false;
	defaultMaterialCI.perceptualRoughness = 0.0f;
	defaultMaterialCI.reflectance = 0.5f;
	defaultMaterialCI.isTransparent = false;
	defaultMaterialCI.textureMask = 0;
	resourceManager.CreateMaterial("White_IronMan", defaultMaterialCI);

	
	scene.AddMeshNode(nullptr, glm::vec3(0.f), glm::vec3(0.f), false, "data/models/ironman.fbx", "White_IronMan");

	scene.AddCameraNode(nullptr, { 0.9f, 1.0f, 3.0f }, glm::radians(glm::vec3(0.f, 0.f, 0.f )), false, 45.f, 0.1f, 500.f, true);
	scene.AddLightNode(nullptr, { 0.0f, 0.0f, -1.0f }, { 0.f, 0.f, 0.f }, false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 1.0f, 1.0f, 1.0f });
	scene.AddLightNode(nullptr, { 0.0f, 0.0f, -1.0f }, { 0.f, 0.f, 0.f }, false, lux::scene::LightType::LIGHT_TYPE_POINT, { 1.0f, 1.0f, 1.0f });

}