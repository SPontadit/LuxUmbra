#include "Engine.h"

void BuildPostProcessScene(lux::Engine& luxUmbra) noexcept;
void BuildDirectionalShadowScene(lux::Engine& luxUmbra) noexcept;
void BuildPBRModels(lux::Engine& luxUmbra) noexcept;
void BuildPBRMaterials(lux::Engine& luxUmbra) noexcept;
void BuildSphereScene(lux::Engine& luxUmbra) noexcept;
void BuildTransparentScene(lux::Engine& luxUmbra) noexcept;


void AddDirectionalLightDebugMesh(lux::scene::Scene& scene, lux::scene::LightNode* light) noexcept;
void AddPointLightDebugMesh(lux::scene::Scene& scene, lux::scene::LightNode* light) noexcept;
void CreateMaterials(lux::Engine& luxUmbra) noexcept;


int main(int ac, char* av[])
{
	lux::Engine luxUmbra;

	luxUmbra.Initialize(1200, 800);

	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();
	resourceManager.UseCubemap("data/envmaps/Ridgecrest_Road_Ref.hdr");

	CreateMaterials(luxUmbra);

	BuildTransparentScene(luxUmbra);
	BuildSphereScene(luxUmbra);
	BuildPostProcessScene(luxUmbra);
	BuildDirectionalShadowScene(luxUmbra);
	BuildPBRModels(luxUmbra);
	BuildPBRMaterials(luxUmbra);

	luxUmbra.Run();

	return 0;
}

void BuildTransparentScene(lux::Engine& luxUmbra) noexcept
{
	lux::scene::Scene& scene = luxUmbra.GetScene(lux::SCENE::TRANSPARENT_SCENE);
	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();
	
	std::shared_ptr<lux::resource::Texture> luxLogo = resourceManager.GetTexture("data/textures/LuxumbraLogo.png");
	std::shared_ptr<lux::resource::Texture> vkLogo = resourceManager.GetTexture("data/textures/Vulkan_Logo.png");

	lux::resource::MaterialCreateInfo luxMaterialCI;
	luxMaterialCI.baseColor = glm::vec3(1.0f);
	luxMaterialCI.albedo = luxLogo;
	luxMaterialCI.metallic = false;
	luxMaterialCI.perceptualRoughness = 0.0f;
	luxMaterialCI.reflectance = 0.0f;
	luxMaterialCI.isUnlit = 1;
	luxMaterialCI.isTransparent = true;
	resourceManager.CreateMaterial("Lux_Logo", luxMaterialCI);

	lux::resource::MaterialCreateInfo vkMaterialCI;
	vkMaterialCI.baseColor = glm::vec3(1.0f);
	vkMaterialCI.albedo = vkLogo;
	vkMaterialCI.metallic = false;
	vkMaterialCI.perceptualRoughness = 0.0f;
	vkMaterialCI.reflectance = 0.0f;
	vkMaterialCI.isTransparent = true;
	vkMaterialCI.isUnlit = 1;
	resourceManager.CreateMaterial("Vk_Logo", vkMaterialCI);

	lux::scene::MeshNode* plane = scene.AddMeshNode(nullptr, glm::vec3(-1.0f, 2.f, 0.f), glm::radians(glm::vec3(0.f, 90.f, 90.f)), false, lux::resource::MeshPrimitive::MESH_PLANE_PRIMITIVE, "Lux_Logo");
	plane->SetLocalScale(glm::vec3(1.0f, 6.0f, 6.0f));

	scene.AddMeshNode(nullptr, glm::vec3(-1.5f, 2.f, 25.0f), glm::radians(glm::vec3(0.f)), false, lux::resource::MeshPrimitive::MESH_CUBE_PRIMITIVE, "Vk_Logo");
	scene.AddMeshNode(nullptr, glm::vec3(-1.5f, 4.f, 25.0f), glm::radians(glm::vec3(0.f)), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Vk_Logo");

	scene.AddCameraNode(nullptr, {0.5f, 3.f, 30.0f }, glm::radians(glm::vec3(0.0f)), false, 45.f, 0.01f, 1000.f, true);

	lux::scene::MeshNode* limit = scene.AddMeshNode(nullptr, glm::vec3(-25.0F, 15.0f, -20.0f), glm::radians(glm::vec3(0.0f)), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Limit");
	limit->SetLocalScale(glm::vec3(0.001f));
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
	size_t column = 6;

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

	row ++;
	for (size_t j = 0; j <= column; j++)
	{
		defaultMaterialCI.perceptualRoughness = TO_FLOAT(j) / TO_FLOAT(column);

		defaultMaterialCI.metallic = true;
		std::string name = ("Metallic_" + std::to_string(j));
		resourceManager.CreateMaterial(name, defaultMaterialCI);
		scene.AddMeshNode(nullptr, { j, row, 0.f }, { 0.0f, 0.0f, 0.0f }, false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, name);
	}

	row ++;
	defaultMaterialCI.baseColor = glm::vec3(0.83137254902f, 0.6862745098f, 0.21568627451f);

	for (size_t j = 0; j <= column; j++)
	{
		defaultMaterialCI.perceptualRoughness = TO_FLOAT(j) / TO_FLOAT(column);

		defaultMaterialCI.metallic = true;
		std::string name = ("Gold_" + std::to_string(j));
		resourceManager.CreateMaterial(name, defaultMaterialCI);
		scene.AddMeshNode(nullptr, { j, row, 0.f }, { 0.0f, 0.0f, 0.0f }, false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, name);
	}

	defaultMaterialCI.baseColor = glm::vec3(1.0f, 0.0f, 0.0f);


	// Clear Coat
	defaultMaterialCI.reflectance = 0.5f;
	defaultMaterialCI.perceptualRoughness = 0.5f;
	row += 2;
	for (size_t j = 0; j <= column; j++)
	{
		defaultMaterialCI.clearCoatPerceptualRoughness = 0.25f;
		defaultMaterialCI.clearCoat = TO_FLOAT(j) / TO_FLOAT(column);

		defaultMaterialCI.metallic = false;
		std::string name = ("ClearCoatLowRough_" + std::to_string(j));
		resourceManager.CreateMaterial(name, defaultMaterialCI);
		scene.AddMeshNode(nullptr, { j, row, 0.f }, { 0.0f, 0.0f, 0.0f }, false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, name);
	}

	row++;
	for (size_t j = 0; j <= column; j++)
	{
		defaultMaterialCI.clearCoatPerceptualRoughness = 1.0f - TO_FLOAT(j) / TO_FLOAT(column);
		defaultMaterialCI.clearCoat = 0.8f;

		defaultMaterialCI.metallic = false;
		std::string name = ("ClearCoatHighRough_" + std::to_string(j));
		resourceManager.CreateMaterial(name, defaultMaterialCI);
		scene.AddMeshNode(nullptr, { j, row, 0.f }, { 0.0f, 0.0f, 0.0f }, false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, name);
	}

	row++;
	for (size_t j = 0; j <= column; j++)
	{
		defaultMaterialCI.clearCoatPerceptualRoughness = 0.2f;
		defaultMaterialCI.clearCoat = TO_FLOAT(j) / TO_FLOAT(column);

		defaultMaterialCI.metallic = true;
		std::string name = ("ClearCoatMetLowRough_" + std::to_string(j));
		resourceManager.CreateMaterial(name, defaultMaterialCI);
		scene.AddMeshNode(nullptr, { j, row, 0.f }, { 0.0f, 0.0f, 0.0f }, false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, name);
	}

	row++;
	for (size_t j = 0; j <= column; j++)
	{
		defaultMaterialCI.clearCoatPerceptualRoughness = 1.0f - TO_FLOAT(j) / TO_FLOAT(column);
		defaultMaterialCI.clearCoat = 0.8f;

		defaultMaterialCI.metallic = true;
		std::string name = ("ClearCoatMetHighRough_" + std::to_string(j));
		resourceManager.CreateMaterial(name, defaultMaterialCI);
		scene.AddMeshNode(nullptr, { j, row, 0.f }, { 0.0f, 0.0f, 0.0f }, false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, name);
	}

	scene.AddLightNode(nullptr, { 0.0f, 0.0f, -1.0f }, glm::radians(glm::vec3(-30.f, 45.f, 0.f)), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 1.0f, 1.0f, 1.0f });

	scene.AddCameraNode(nullptr, { 3.f, 2.5f, 6.0f }, { 0.f, 0.f, 0.f }, false, 45.f, 0.01f, 1000.f, true);
}

void BuildDirectionalShadowScene(lux::Engine& luxUmbra) noexcept
{
	lux::scene::Scene& scene = luxUmbra.GetScene(lux::SCENE::DIRECTIONAL_SHADOW_SCENE);

	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();

	std::shared_ptr<lux::resource::Texture> ironmanDif = resourceManager.GetTexture("data/textures/ironman.dff.png");
	std::shared_ptr<lux::resource::Texture> ironmanNrm = resourceManager.GetTexture("data/textures/ironman.norm.png");

	lux::resource::MaterialCreateInfo defaultMaterialCI;
	defaultMaterialCI.baseColor = glm::vec3(1.0f);
	defaultMaterialCI.metallic = false;
	defaultMaterialCI.perceptualRoughness = 0.0f;
	defaultMaterialCI.reflectance = 0.0f;
	defaultMaterialCI.isTransparent = false;
	resourceManager.CreateMaterial("Shadow_Unlit_White", defaultMaterialCI);


	defaultMaterialCI.albedo = ironmanDif;
	defaultMaterialCI.isTransparent = false;
	defaultMaterialCI.normal = ironmanNrm;
	resourceManager.CreateMaterial("ironman", defaultMaterialCI);

	scene.AddCameraNode(nullptr, { 5.f, 4.f, 8.5f }, glm::radians(glm::vec3(-20.0f, 45.0f, 0.0f)), false, 45.f, 0.01f, 1000.f, true);

	glm::vec3 planeScale = glm::vec3(2.0f);

	lux::scene::MeshNode* plane = scene.AddMeshNode(nullptr, glm::vec3(-1.0f, 2.f, 0.f), glm::radians(glm::vec3(0.f, 90.f, 0.f)), false, lux::resource::MeshPrimitive::MESH_PLANE_PRIMITIVE, "White_Plane");
	plane->SetLocalScale(planeScale);

	plane = scene.AddMeshNode(nullptr, glm::vec3(-3.5f, 2.f, 2.5f), glm::radians(glm::vec3(0.f, 180.f, 0.f)), false, lux::resource::MeshPrimitive::MESH_PLANE_PRIMITIVE, "White_Plane");
	plane->SetLocalScale(planeScale);

	plane = scene.AddMeshNode(nullptr, glm::vec3(-1.f, -0.5f, 2.5f), glm::radians(glm::vec3(0.f, 0.f, -90.f)), false, lux::resource::MeshPrimitive::MESH_PLANE_PRIMITIVE, "White_Plane");
	plane->SetLocalScale(planeScale);

	scene.AddMeshNode(nullptr, glm::vec3(-0.5f, 0.0f, 3.0f), glm::vec3(0.f), false, "data/models/ironman.fbx", "ironman");

	lux::scene::LightNode* light = scene.AddLightNode(nullptr, { -0.5f, 1.0f, 4.0f }, { 0.f, 0.f, 0.f }, false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 1.0f, 1.0f, 1.0f });
	AddDirectionalLightDebugMesh(scene, light);

	light = scene.AddLightNode(nullptr, { 0.75f, 1.0f, 3.0f }, glm::radians(glm::vec3(0.f, 90.f, 0.f)) , false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, { 1.0f, 1.0f, 1.0f });
	AddDirectionalLightDebugMesh(scene, light);


	lux::scene::MeshNode* limit = scene.AddMeshNode(nullptr, glm::vec3(3.0f, 7.0f, -2.0f), glm::radians(glm::vec3(0.0f)), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Limit");
	limit->SetLocalScale(glm::vec3(0.001f));

	limit = scene.AddMeshNode(nullptr, glm::vec3(-5.0f, -2.0f, 7.0f), glm::radians(glm::vec3(0.0f)), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Limit");
	limit->SetLocalScale(glm::vec3(0.001f));
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

	scene.AddCameraNode(nullptr, { -0.2f, 1.9f, 5.f }, glm::radians(glm::vec3(-14.f, -25.5f, 0.f)), false, 45.f, 0.1f, 50.f, true);

	lux::scene::MeshNode* DamagedHelmet = scene.AddMeshNode(nullptr, glm::vec3(-3.0f, 0.5f, 0.f), glm::radians(glm::vec3(0.0f, 30.0f, 0.0f)), false, "data/DamagedHelmet/DamagedHelmet.gltf", "Helmet");
	DamagedHelmet->SetLocalScale(glm::vec3(0.6f));

	lux::scene::MeshNode* sciFiHelmet = scene.AddMeshNode(nullptr, glm::vec3(3.2f, 0.5f, 0.5f), glm::radians(glm::vec3(0.0f, -30.0f, 0.0f)), false, "data/SciFiHelmet/SciFiHelmet.gltf", "SciFiHelmet");
	sciFiHelmet->SetLocalScale(glm::vec3(0.5f));

	lux::scene::MeshNode* corset = scene.AddMeshNode(nullptr, glm::vec3(0.0f, -0.5f, -2.0f), glm::vec3(0.f), false, "data/Corset/Corset.gltf", "Corset");
	corset->SetLocalScale(glm::vec3(35.0f));

	lux::scene::MeshNode* plan = scene.AddMeshNode(nullptr, glm::vec3(0.0f, -0.5f, 0.0f), glm::radians(glm::vec3(0.0f, 0.0f, -90.0f)), false, lux::resource::MeshPrimitive::MESH_PLANE_PRIMITIVE, "White_Plane");
	plan->SetLocalScale(glm::vec3(5.0f));

	//lux::scene::MeshNode* area = scene.AddMeshNode(nullptr, glm::vec3(0.0f, -0.5f, 0.0f), glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)), false, "data/models/test_area.fbx", "White_Plane");


	lux::scene::LightNode* pointLight = scene.AddLightNode(nullptr, glm::vec3(-2.2f, 1.8f, 0.0f), glm::radians(glm::vec3(0.f, 0.f, 0.f)), false, lux::scene::LightType::LIGHT_TYPE_POINT, glm::vec3(1.f, 0.f, 0.f));
	AddPointLightDebugMesh(scene, pointLight);
	pointLight->SetRadius(10.0f);

	pointLight = scene.AddLightNode(nullptr, glm::vec3(-3.5f, 1.8f, 0.75f), glm::radians(glm::vec3(0.f, 0.f, 0.f)), false, lux::scene::LightType::LIGHT_TYPE_POINT, glm::vec3(0.000f, 0.941f, 1.000f));
	AddPointLightDebugMesh(scene, pointLight);
	pointLight->SetRadius(10.0f);

	pointLight = scene.AddLightNode(nullptr, glm::vec3(2.0f, 2.5f, -0.5f), glm::radians(glm::vec3(0.f, 0.f, 0.f)), false, lux::scene::LightType::LIGHT_TYPE_POINT, glm::vec3(1.000f, 0.737f, 0.255f));
	AddPointLightDebugMesh(scene, pointLight);
	pointLight->SetRadius(20.0f);

	lux::scene::MeshNode* limit = scene.AddMeshNode(nullptr, glm::vec3(5.5f, 3.0f, -5.0f), glm::radians(glm::vec3(0.0f)), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Limit");
	limit->SetLocalScale(glm::vec3(0.001f));

	limit = scene.AddMeshNode(nullptr, glm::vec3(-5.0f, -2.0f, 7.0f), glm::radians(glm::vec3(0.0f)), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Limit");
	limit->SetLocalScale(glm::vec3(0.001f));
}

void BuildPBRMaterials(lux::Engine& luxUmbra) noexcept
{
	lux::scene::Scene& scene = luxUmbra.GetScene(lux::SCENE::PBR_MATERIALS_SCENE);
	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();

	std::shared_ptr<lux::resource::Texture> albedoBloody = resourceManager.GetTexture("data/textures/BloodyGuts_basecolor.jpg");
	std::shared_ptr<lux::resource::Texture> normalBloody = resourceManager.GetTexture("data/textures/BloodyGuts_normal.png");
	std::shared_ptr<lux::resource::Texture> metRoughBloody = resourceManager.GetTexture("data/textures/BloodyGuts_roughness.png");
	std::shared_ptr<lux::resource::Texture> aoBloody = resourceManager.GetTexture("data/textures/BloodyGuts_AO.png");

	std::shared_ptr<lux::resource::Texture> albedoBlueGranite = resourceManager.GetTexture("data/textures/Blue_Granite_BaseColor.jpg");
	std::shared_ptr<lux::resource::Texture> metRoughBlueGranite = resourceManager.GetTexture("data/textures/Blue_Granite_Metallic_Roughness.png");

	std::shared_ptr<lux::resource::Texture> albedoMarble = resourceManager.GetTexture("data/textures/Marble_BaseColor.jpg");
	std::shared_ptr<lux::resource::Texture> normalMarble = resourceManager.GetTexture("data/textures/Marble_Normal.png");
	std::shared_ptr<lux::resource::Texture> roughMarble = resourceManager.GetTexture("data/textures/Marble_Roughness.png");

	std::shared_ptr<lux::resource::Texture> albedoWallpaper = resourceManager.GetTexture("data/textures/Wallpaper_BaseColor.jpg");
	std::shared_ptr<lux::resource::Texture> normalWallpaper = resourceManager.GetTexture("data/textures/Wallpaper_Normal.jpg");
	std::shared_ptr<lux::resource::Texture> metRoughWallpaper = resourceManager.GetTexture("data/textures/Wallpaper_Metallic_Roughness.png");
	std::shared_ptr<lux::resource::Texture> aoWallpaper = resourceManager.GetTexture("data/textures/Wallpaper_AO.png");

	std::shared_ptr<lux::resource::Texture> albedoTriangle = resourceManager.GetTexture("data/textures/Triangle_BaseColor.jpg");
	std::shared_ptr<lux::resource::Texture> normalTriangle = resourceManager.GetTexture("data/textures/Triangle_Normal.png");
	std::shared_ptr<lux::resource::Texture> metRoughTriangle = resourceManager.GetTexture("data/textures/Triangle_Metallic_Roughness.png");
	std::shared_ptr<lux::resource::Texture> aoTriangle = resourceManager.GetTexture("data/textures/Triangle_AO.png");

	lux::resource::MaterialCreateInfo BloodyGutsMaterialCI = {};
	BloodyGutsMaterialCI.baseColor = glm::vec3(1.0f);
	BloodyGutsMaterialCI.albedo = albedoBloody;
	BloodyGutsMaterialCI.normal = normalBloody;
	BloodyGutsMaterialCI.metallicRoughness = metRoughBloody;
	BloodyGutsMaterialCI.ambientOcclusion = aoBloody;
	BloodyGutsMaterialCI.textureMask = lux::resource::TextureMask::ROUGHNESS_TEXTURE_MASK;
	BloodyGutsMaterialCI.reflectance = 0.2f;
	BloodyGutsMaterialCI.isTransparent = false;
	BloodyGutsMaterialCI.metallic = 0.0f;
	BloodyGutsMaterialCI.clearCoat = 0.0f;
	BloodyGutsMaterialCI.clearCoatPerceptualRoughness = 0.0f;
	resourceManager.CreateMaterial("BloodyGuts", BloodyGutsMaterialCI);


	lux::resource::MaterialCreateInfo BlueGraniteMaterialCI = {};
	BlueGraniteMaterialCI.baseColor = glm::vec3(1.0f);
	BlueGraniteMaterialCI.albedo = albedoBlueGranite;
	BlueGraniteMaterialCI.metallicRoughness = metRoughBlueGranite;
	BlueGraniteMaterialCI.textureMask = lux::resource::TextureMask::ROUGHNESS_TEXTURE_MASK | lux::resource::TextureMask::METALLIC_TEXTURE_MASK;
	BlueGraniteMaterialCI.reflectance = 0.6f;
	BlueGraniteMaterialCI.isTransparent = false;
	BlueGraniteMaterialCI.clearCoat = 0.2f;
	BlueGraniteMaterialCI.clearCoatPerceptualRoughness = 0.8f;
	resourceManager.CreateMaterial("Blue_Granite", BlueGraniteMaterialCI);


	lux::resource::MaterialCreateInfo MarbleMaterialCI = {};
	MarbleMaterialCI.baseColor = glm::vec3(1.0f);
	MarbleMaterialCI.albedo = albedoMarble;
	MarbleMaterialCI.normal = normalMarble;
	MarbleMaterialCI.metallicRoughness = roughMarble;
	MarbleMaterialCI.textureMask = lux::resource::TextureMask::ROUGHNESS_TEXTURE_MASK ;
	MarbleMaterialCI.reflectance = 0.85f;
	MarbleMaterialCI.isTransparent = false;
	MarbleMaterialCI.metallic = 0.0f;
	MarbleMaterialCI.clearCoat = 0.0f;
	MarbleMaterialCI.clearCoatPerceptualRoughness = 0.0;
	resourceManager.CreateMaterial("Marble", MarbleMaterialCI);


	lux::resource::MaterialCreateInfo WallpaperMaterialCI = {};
	WallpaperMaterialCI.baseColor = glm::vec3(1.0f);
	WallpaperMaterialCI.albedo = albedoWallpaper;
	WallpaperMaterialCI.normal = normalWallpaper;
	WallpaperMaterialCI.metallicRoughness = metRoughWallpaper;
	WallpaperMaterialCI.ambientOcclusion = aoWallpaper;
	WallpaperMaterialCI.textureMask = lux::resource::TextureMask::ROUGHNESS_TEXTURE_MASK | lux::resource::TextureMask::METALLIC_TEXTURE_MASK;;
	WallpaperMaterialCI.reflectance = 0.0f;
	WallpaperMaterialCI.isTransparent = false;
	WallpaperMaterialCI.clearCoat = 0.0f;
	WallpaperMaterialCI.clearCoatPerceptualRoughness = 0.0f;
	resourceManager.CreateMaterial("Wallpaper", WallpaperMaterialCI);


	lux::resource::MaterialCreateInfo TriangleMaterialCI = {};
	TriangleMaterialCI.baseColor = glm::vec3(1.0f);
	TriangleMaterialCI.albedo = albedoTriangle;
	TriangleMaterialCI.normal = normalTriangle;
	TriangleMaterialCI.metallicRoughness = metRoughTriangle;
	TriangleMaterialCI.ambientOcclusion = aoTriangle;
	TriangleMaterialCI.textureMask = lux::resource::TextureMask::ROUGHNESS_TEXTURE_MASK | lux::resource::TextureMask::METALLIC_TEXTURE_MASK;;
	TriangleMaterialCI.reflectance = 0.0f;
	TriangleMaterialCI.isTransparent = false;
	TriangleMaterialCI.clearCoat = 0.0f;
	TriangleMaterialCI.clearCoatPerceptualRoughness = 0.0f;
	resourceManager.CreateMaterial("Triangle", TriangleMaterialCI);


	scene.AddCameraNode(nullptr, { 1.f, 3.f, 4.f }, glm::radians(glm::vec3(-30.0f, 15.0f, 0.0f)), false, 45.f, 0.01f, 50.f, true);

	glm::vec3 rotation = glm::vec3(-90.0f, 40.0f, 0.0f);

	scene.AddMeshNode(nullptr, glm::vec3(2.0f, 0.f, 0.f), glm::radians(rotation), false, lux::resource::MeshPrimitive::MESH_PREVIEW_MATERIAL_PRIMITIVE, "Wallpaper");
	scene.AddMeshNode(nullptr, glm::vec3(1.0f, 0.f, 0.f), glm::radians(rotation), false, lux::resource::MeshPrimitive::MESH_PREVIEW_MATERIAL_PRIMITIVE, "BloodyGuts");
	scene.AddMeshNode(nullptr, glm::vec3(0.0f, 0.f, 0.f), glm::radians(rotation), false, lux::resource::MeshPrimitive::MESH_PREVIEW_MATERIAL_PRIMITIVE, "Triangle");
	scene.AddMeshNode(nullptr, glm::vec3(-1.0f, 0.f, 0.f), glm::radians(rotation), false, lux::resource::MeshPrimitive::MESH_PREVIEW_MATERIAL_PRIMITIVE, "Blue_Granite");
	scene.AddMeshNode(nullptr, glm::vec3(-2.0f, 0.f, 0.f), glm::radians(rotation), false, lux::resource::MeshPrimitive::MESH_PREVIEW_MATERIAL_PRIMITIVE, "Marble");

	lux::scene::MeshNode* plane = scene.AddMeshNode(nullptr, glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(glm::vec3(0.0f, 0.0f, -90.0f)), false, lux::resource::MeshPrimitive::MESH_PLANE_PRIMITIVE, "White_Plane");
	plane->SetLocalScale(glm::vec3(2.0f));

	lux::scene::LightNode* lightNode = scene.AddLightNode(nullptr, glm::vec3(2.0f, 3.f, 2.f), glm::radians(glm::vec3(-55.0f, 50.0f, 0.0f)), false, lux::scene::LightType::LIGHT_TYPE_DIRECTIONAL, glm::vec3(1.f, 1.f, 1.f));
	AddDirectionalLightDebugMesh(scene, lightNode);

	lux::scene::MeshNode* limit = scene.AddMeshNode(nullptr, glm::vec3(3.0f, 4.5f, -5.0f), glm::radians(glm::vec3(0.0f)), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Limit");
	limit->SetLocalScale(glm::vec3(0.001f));

	limit = scene.AddMeshNode(nullptr, glm::vec3(-5.5f, -8.0f, 4.0f), glm::radians(glm::vec3(0.0f)), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Limit");
	limit->SetLocalScale(glm::vec3(0.001f));

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

void AddDirectionalLightDebugMesh(lux::scene::Scene& scene, lux::scene::LightNode* light) noexcept
{
	glm::vec3 debugLightScale = glm::vec3(0.05f, 0.05f, 0.3f);

	lux::scene::MeshNode* debugLight = scene.AddMeshNode(light, glm::vec3(0.0f), glm::vec3(0.0f), false, lux::resource::MeshPrimitive::MESH_CUBE_PRIMITIVE, "Light_Debug");
	debugLight->SetLocalScale(debugLightScale);
	debugLight->SetIsCastingShadow(false);
}

void AddPointLightDebugMesh(lux::scene::Scene& scene, lux::scene::LightNode* light) noexcept
{
	glm::vec3 debugLightScale = glm::vec3(0.2f);

	lux::scene::MeshNode* debugLight = scene.AddMeshNode(light, glm::vec3(0.0f), glm::vec3(0.0f), false, lux::resource::MeshPrimitive::MESH_SPHERE_PRIMITIVE, "Light_Debug");
	debugLight->SetLocalScale(debugLightScale);
	debugLight->SetIsCastingShadow(false);
}

void CreateMaterials(lux::Engine& luxUmbra) noexcept
{
	lux::resource::ResourceManager& resourceManager = luxUmbra.GetResourceManager();

	lux::resource::MaterialCreateInfo lightDebugCI;
	lightDebugCI.baseColor = glm::vec3(1.000f, 0.941f, 0.333f);
	lightDebugCI.isUnlit = 1;
	lightDebugCI.isTransparent = false;
	resourceManager.CreateMaterial("Light_Debug", lightDebugCI);

	lux::resource::MaterialCreateInfo dummyCI;
	dummyCI.baseColor = glm::vec3(0.0f);
	dummyCI.isUnlit = 1;
	dummyCI.isTransparent = false;
	resourceManager.CreateMaterial("Limit", dummyCI);


	lux::resource::MaterialCreateInfo whitePlaneCI;
	whitePlaneCI.baseColor = glm::vec3(1.0f);
	whitePlaneCI.metallic = false;
	whitePlaneCI.perceptualRoughness = 0.0f;
	whitePlaneCI.reflectance = 0.0f;
	whitePlaneCI.isTransparent = false;
	resourceManager.CreateMaterial("White_Plane", whitePlaneCI);
}