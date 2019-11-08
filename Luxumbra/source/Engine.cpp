#include "Engine.h"

#include <set>

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_vulkan.h"

#include "glm\gtc\type_ptr.hpp"

namespace lux
{

	Engine::Engine() noexcept
		: isInitialized(false), window(), rhi(), scenes(), resourceManager(rhi),
		currentScene(0), drawGUI(true)
	{

	}

	Engine::~Engine()
	{
		rhi.WaitIdle();
	}

	bool Engine::Initialize(uint32_t windowWidth, uint32_t windowHeight) noexcept
	{
		if (!window.Initialize(windowWidth, windowHeight))
			return false;

		if (!rhi.Initialize(window))
			return false;


		resourceManager.Initialize();

		for (size_t i = 0; i < scenes.size(); i++)
			scenes[i].Initialize(window, rhi, resourceManager);

		isInitialized = true;


		return true;
	}

	void Engine::Run() noexcept
	{
		currentScene = 0;

		bool inputExit = false;

		while (!window.ShouldClose() && !inputExit)
		{
			window.PollEvents();

			inputExit = window.GetActionsStatus(Action::EXIT);

			float deltaTime = window.GetDeltaTime();

			scene::Scene& scene = scenes[TO_SIZE_T(currentScene)];

			if (window.GetHasFocus())
			{
				Update(scene, deltaTime);

				DrawImgui(scene, deltaTime);

				rhi.Render(scene.GetCurrentCamera(), scene.GetMeshNodes(), scene.GetLightNodes());
			}
		}
	}

	scene::Scene& Engine::GetScene(SCENE scene) noexcept
	{
		return scenes[static_cast<int32_t>(scene)];
	}

	resource::ResourceManager& Engine::GetResourceManager() noexcept
	{
		return resourceManager;
	}

	void Engine::Update(scene::Scene& scene, float deltaTime) noexcept
	{
		scene::CameraNode* camera = scene.GetCurrentCamera();
		float moveDelta = 3.f * deltaTime;

		if (window.GetActionsStatus(Action::FREE_LOOK))
		{
			float mouseXDelta, mouseYDelta;
			window.GetMouseDelta(mouseXDelta, mouseYDelta);

			glm::vec3 cameraRotation = camera->GetLocalRotation();

			cameraRotation.y += glm::radians(-mouseXDelta) * 0.25f;
			cameraRotation.x += glm::radians(-mouseYDelta) * 0.25f;

			camera->SetLocalRotation(cameraRotation);
		}

		if (window.GetActionsStatus(Action::FORWARD))
		{
			glm::vec3 cameraPosition = camera->GetLocalPosition();
			glm::vec3 cameraForward = glm::rotate(glm::quat(camera->GetLocalRotation()), glm::vec3(0.f, 0.f, -1.f));

			cameraPosition += cameraForward * moveDelta;

			camera->SetLocalPosition(cameraPosition);
		}

		if (window.GetActionsStatus(Action::BACKWARD))
		{
			glm::vec3 cameraPosition = camera->GetLocalPosition();
			glm::vec3 cameraBackward = glm::rotate(glm::quat(camera->GetLocalRotation()), glm::vec3(0.f, 0.f, 1.f));

			cameraPosition += cameraBackward * moveDelta;

			camera->SetLocalPosition(cameraPosition);
		}

		if (window.GetActionsStatus(Action::RIGHT))
		{
			glm::vec3 cameraPosition = camera->GetLocalPosition();
			glm::vec3 cameraRight = glm::rotate(glm::quat(camera->GetLocalRotation()), glm::vec3(1.f, 0.f, 0.f));

			cameraPosition += cameraRight * moveDelta;

			camera->SetLocalPosition(cameraPosition);
		}

		if (window.GetActionsStatus(Action::LEFT))
		{
			glm::vec3 cameraPosition = camera->GetLocalPosition();
			glm::vec3 cameraLeft = glm::rotate(glm::quat(camera->GetLocalRotation()), glm::vec3(-1.f, 0.f, 0.f));

			cameraPosition += cameraLeft * moveDelta;

			camera->SetLocalPosition(cameraPosition);
		}

		if (window.GetActionsStatus(Action::UP))
		{
			glm::vec3 cameraPosition = camera->GetLocalPosition();
			glm::vec3 cameraUp = glm::rotate(glm::quat(camera->GetLocalRotation()), glm::vec3(0.f, 1.f, 0.f));

			cameraPosition += cameraUp * moveDelta;

			camera->SetLocalPosition(cameraPosition);
		}

		if (window.GetActionsStatus(Action::DOWN))
		{
			glm::vec3 cameraPosition = camera->GetLocalPosition();
			glm::vec3 cameraDown = glm::rotate(glm::quat(camera->GetLocalRotation()), glm::vec3(0.f, -1.f, -0.f));

			cameraPosition += cameraDown * moveDelta;

			camera->SetLocalPosition(cameraPosition);
		}

		static bool lastToggleUI = true;
		bool toggleUI = window.GetActionsStatus(Action::TOGGLE_UI);

		if (toggleUI && !lastToggleUI)
			drawGUI = !drawGUI;

		lastToggleUI = toggleUI;
	}

	void Engine::DrawImgui(scene::Scene& scene, float deltaTime) noexcept
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (drawGUI)
		{
			float framerate = 1.f / deltaTime;

			char buf[128];
			sprintf_s(buf, "Luxumbra Engine %d###Luxumbra Engine", (int)framerate);

			ImGui::Begin(buf);

			ImGui::RadioButton("Sphere Scene", &currentScene, TO_INT32_T(SCENE::SPHERE_SCENE));
			ImGui::RadioButton("PostProcess Scene", &currentScene, TO_INT32_T(SCENE::POST_PROCESS_SCENE));
			ImGui::RadioButton("PBR Models Scene", &currentScene, TO_INT32_T(SCENE::PBR_MODELS_SCENE));
			ImGui::RadioButton("PBR Materials Scene", &currentScene, TO_INT32_T(SCENE::PBR_MATERIALS_SCENE));
			//ImGui::RadioButton("Transparent Scene", &currentScene, TO_INT32_T(SCENE::TRANSPARENT_SCENE));
			ImGui::RadioButton("Shadow Scene", &currentScene, TO_INT32_T(SCENE::DIRECTIONAL_SHADOW_SCENE));

			if (ImGui::Button("Reload Shader"))
			{
				rhi.RebuildPipelines();
			}

			ImGui::NewLine();

			if (ImGui::BeginTabBar("##TabBar"))
			{
				if (ImGui::BeginTabItem("Post Process"))
				{
					lux::rhi::PostProcessParameters& postProcess = rhi.forward.postProcessParameters;

					if (ImGui::CollapsingHeader("Debug:", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::Spacing();

						int splitViewMask = postProcess.splitViewMask;

						bool showToneMapping = splitViewMask & lux::rhi::PostProcessSplitViewMask::SPLIT_VIEW_TONE_MAPPING_MASK;
						bool showFXAA = splitViewMask & lux::rhi::PostProcessSplitViewMask::SPLIT_VIEW_FXAA_MASK;
						bool showSSAO = splitViewMask & lux::rhi::PostProcessSplitViewMask::SPLIT_VIEW_SSAO_MASK;
						bool showSSAOOnly = splitViewMask & lux::rhi::PostProcessSplitViewMask::SPLIT_VIEW_SSAO_ONLY_MASK;
						bool newShowToneMapping = showToneMapping;
						bool newShowFXAA = showFXAA;
						bool newShowSSAO = showSSAO;
						bool newShowSSAOOnly = showSSAOOnly;

						ImGui::Checkbox("Show Tone Mapping", &newShowToneMapping);
						ImGui::Checkbox("Show FXAA", &newShowFXAA);
						ImGui::Checkbox("Show SSAO", &newShowSSAO);
						ImGui::Checkbox("Show SSAO Only", &newShowSSAOOnly);

						int newSplitViewMask = 0;

						newSplitViewMask |= newShowToneMapping ? lux::rhi::PostProcessSplitViewMask::SPLIT_VIEW_TONE_MAPPING_MASK : 0;
						newSplitViewMask |= newShowFXAA ? lux::rhi::PostProcessSplitViewMask::SPLIT_VIEW_FXAA_MASK : 0;
						newSplitViewMask |= newShowSSAO ? lux::rhi::PostProcessSplitViewMask::SPLIT_VIEW_SSAO_MASK : 0;
						newSplitViewMask |= newShowSSAOOnly ? lux::rhi::PostProcessSplitViewMask::SPLIT_VIEW_SSAO_ONLY_MASK : 0;

						if (newSplitViewMask != splitViewMask)
							postProcess.splitViewMask = newSplitViewMask;

						if (newSplitViewMask != 0)
						{
							ImGui::Spacing();

							// FXAA Percent
							float splitViewRatio = postProcess.splitViewRatio;
							float newSplitViewRatio = splitViewRatio;
							ImGui::SliderFloat("Percent", &newSplitViewRatio, 0.0f, 1.0f, "%.2f");

							if (newSplitViewRatio != splitViewRatio)
								postProcess.splitViewRatio = newSplitViewRatio;
						}

						ImGui::NewLine();
					}

					if (ImGui::CollapsingHeader("ToneMapping:", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::Spacing();

						int toneMapping = postProcess.toneMapping;
						int newToneMapping = toneMapping;
						ImGui::Combo("Tone Mapping", &newToneMapping, "ACES Film\0Uncharted 2\0Reinhard");

						if (newToneMapping != toneMapping)
							postProcess.toneMapping = newToneMapping;

						ImGui::Spacing();

						// Exposure
						float exposure = postProcess.exposure;
						float newExposure = exposure;
						ImGui::SliderFloat("Exposure", &newExposure, 0.0f, 10.0f, "%.2f");

						if (newExposure != exposure)
							postProcess.exposure = newExposure;

						ImGui::NewLine();
					}

					if (ImGui::CollapsingHeader("FXAA:", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::Spacing();

						int FXAAQuality;
						if (postProcess.FXAARelativeThreshold == 0.250f) FXAAQuality = 0;
						else if (postProcess.FXAARelativeThreshold == 0.166f) FXAAQuality = 1;
						else if (postProcess.FXAARelativeThreshold == 0.125f) FXAAQuality = 2;

						int newFXAAQuality = FXAAQuality;

						const char* qualityName[3] = { "Low", "Medium", "High" };
						ImGui::SliderInt("Quality", &newFXAAQuality, 0, 2, qualityName[newFXAAQuality]);

						if (newFXAAQuality != FXAAQuality)
						{
							// Low
							if (newFXAAQuality == 0)
							{
								postProcess.FXAAContrastThreshold = 0.0833f;
								postProcess.FXAARelativeThreshold = 0.250f;

							}
							// Medium
							else if (newFXAAQuality == 1)
							{
								postProcess.FXAAContrastThreshold = 0.0625f;
								postProcess.FXAARelativeThreshold = 0.166f;

							}
							// High
							else
							{
								postProcess.FXAAContrastThreshold = 0.0312f;
								postProcess.FXAARelativeThreshold = 0.125f;
							}
						}

						ImGui::NewLine();
					}

					if (ImGui::CollapsingHeader("SSAO:", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::Spacing();

						lux::rhi::SSAOParameters& ssaoParameters = rhi.forward.ssaoParameters;

						// Kernel Size
						int kernelSize = ssaoParameters.kernelSize;
						//int newKernelSize = kernelSize;
						//ImGui::SliderInt("KernelSize", &newKernelSize, 2, 32);
						int kernelSizeID = static_cast<int>(log2(kernelSize)) - 2;
						ImGui::Combo("Kernel Size", &kernelSizeID, " 4 \0 8 \0 16 \0 32");
						int newKernelSize = static_cast<int>(pow(2, kernelSizeID + 2));

						if (newKernelSize != kernelSize)
							ssaoParameters.kernelSize = newKernelSize;


						// Kernel Radius
						float kernelRadius = ssaoParameters.kernelRadius;
						float newKernelRadius = kernelRadius;
						ImGui::SliderFloat("Kernel Radius", &newKernelRadius, 0.2f, 2.0f, "%.2f", 0.5f);

						if (newKernelRadius != kernelRadius)
							ssaoParameters.kernelRadius = newKernelRadius;


						// Bias
						float bias = ssaoParameters.bias;
						float newBias = bias;
						ImGui::SliderFloat("Bias", &newBias, 0.0f, 0.05f, "%.3f", 0.5f);

						if (newBias != bias)
							ssaoParameters.bias = newBias;


						// Strenght
						float strenght = ssaoParameters.strenght;
						float newStrenght = strenght;
						ImGui::SliderFloat("Strenght", &newStrenght, 0.0f, 5.0f, "%.2f");

						if (newStrenght != strenght)
							ssaoParameters.strenght = newStrenght;

						ImGui::NewLine();
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Render Settings"))
				{
					if (ImGui::CollapsingHeader("Shadow Mapping", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::Spacing();

						float depthBiasConstantFactor = rhi.GetShadowMappingDepthBiasConstantFactor();
						float depthBiasSlopeFactor = rhi.GetShadowMappingDepthBiasSlopeFactor();

						if (ImGui::DragFloat("Depth bias constant factor", &depthBiasConstantFactor, 0.01f))
							rhi.SetShadowMappingDepthBiasConstantFactor(depthBiasConstantFactor);

						if (ImGui::DragFloat("Depth bias slope factor", &depthBiasSlopeFactor, 0.01f))
							rhi.SetShadowMappingDepthBiasSlopeFactor(depthBiasSlopeFactor);
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Scene"))
				{
					scene::CameraNode* camera = scene.GetCurrentCamera();
					DisplayCameraNode(camera);
					ImGui::NewLine();

					DisplayMeshNodes(scene.GetMeshNodes());
					ImGui::NewLine();
					DisplayLightNodes(scene.GetLightNodes());
					ImGui::NewLine();

					DisplayMaterials(scene.GetMeshNodes());

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}


			ImGui::End();
		}

		ImGui::Render();
	}

	void Engine::DisplayCameraNode(scene::CameraNode* node) noexcept
	{
		if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DisplayNode(node);

			// TODO: Expose Camera relevant stuff
			
			ImGui::TreePop();
		}
	}

	void Engine::DisplayMeshNodes(const std::vector<scene::MeshNode*>& meshes) noexcept
	{
		size_t meshCount = meshes.size();
		scene::MeshNode* currentMesh;

		if (ImGui::TreeNodeEx("Meshes", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (size_t i = 0; i < meshCount; i++)
			{
				std::string displayName("Mesh " + std::to_string(i) + " | " + meshes[i]->GetMaterial().name);
				if (ImGui::TreeNode(displayName.c_str()))
				{
					currentMesh = meshes[i];
					DisplayNode(currentMesh);

					ImGui::NewLine();

					bool castShadow = currentMesh->GetIsCastingShadow();
					bool newCastShadow = castShadow;
					ImGui::Checkbox("Cast Shadow", &newCastShadow);

					if (castShadow != newCastShadow)
						currentMesh->SetIsCastingShadow(newCastShadow);


					ImGui::TreePop();
				}
			}
			
			ImGui::TreePop();
		}
	}

	void Engine::DisplayLightNodes(const std::vector<scene::LightNode*>& lights) noexcept
	{
		size_t lightCount = lights.size();
		scene::LightNode* currentLight;

		if (ImGui::TreeNodeEx("Lights", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (size_t i = 0; i < lightCount; i++)
			{
				currentLight = lights[i];

				const char* lightType;

				if (currentLight->GetType() == lux::scene::LightType::LIGHT_TYPE_POINT)
					lightType = "Point Light ";
				else
					lightType = "Directional Light ";

				if (ImGui::TreeNode((lightType + std::to_string(i)).c_str()))
				{
					DisplayNode(currentLight);

					glm::vec3 color = currentLight->GetColor();
					ImGui::ColorEdit3("Color", glm::value_ptr(color));
					
					if (color != currentLight->GetColor())
					{
						currentLight->SetColor(color);
					}

					if (currentLight->GetType() == lux::scene::LightType::LIGHT_TYPE_POINT)
					{
						float radius = currentLight->GetRadius();
						float newRadius = radius;

						ImGui::DragFloat("Radius", &newRadius, 0.1f, 0.0f, 100.0f, "%.2f");

						if (newRadius != radius)
							currentLight->SetRadius(newRadius);
					}

					ImGui::TreePop();
				}
			}
		
			ImGui::TreePop();
		}
	}

	void Engine::DisplayMaterials(const std::vector<scene::MeshNode*>& meshes) noexcept
	{
		if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_DefaultOpen))
		{
			std::set<std::string> materialNames;
			for (size_t i = 0; i < meshes.size(); i++)
			{
				resource::Material* currentMaterial = &meshes[i]->GetMaterial();
				std::set<std::string>::iterator mat = materialNames.find(currentMaterial->name);

				if (mat == materialNames.end())
				{
					materialNames.insert(currentMaterial->name);

					if (ImGui::TreeNode(currentMaterial->name.c_str()))
					{
						resource::MaterialParameters& matParameters = currentMaterial->parameter;

						ImGui::ColorEdit3("Base Color", glm::value_ptr(matParameters.baseColor));
						bool metallic = (bool)matParameters.metallic;
						ImGui::Checkbox("Metallic", &metallic);
						matParameters.metallic = metallic;

						if (matParameters.metallic == 0.f)
						{
							ImGui::SliderFloat("Reflectance", &matParameters.reflectance, 0.0f, 1.0f, "%.3f");
						}

						ImGui::SliderFloat("Roughness", &matParameters.perceptualRoughness, 0.045f, 1.0f, "%.3f");

						ImGui::SliderFloat("Clear Coat", &matParameters.clearCoat, 0.0f, 1.0f, "%.3f");

						ImGui::SliderFloat("Cleat Coat Roughness", &matParameters.clearCoatPerceptualRoughness, 0.045f, 1.0f, "%.3f");

						ImGui::TreePop();
					}
				}
			}
			ImGui::TreePop();
		}
	}

	void Engine::DisplayNode(scene::Node* node) noexcept
	{
		//Position
		glm::vec3 position = node->GetLocalPosition();

		ImGui::DragFloat3("Pos", glm::value_ptr(position), 0.01f);

		if (position != node->GetLocalPosition())
		{
			node->SetLocalPosition(position);
		}

		// Rotation
		glm::vec3 rotation = glm::degrees(node->GetLocalRotation());

		ImGui::DragFloat3("Rot", glm::value_ptr(rotation));

		if (rotation != node->GetLocalRotation())
		{
			node->SetLocalRotation(glm::radians(rotation));
		}

		// Rotation
		glm::vec3 scale = node->GetLocalScale();

		ImGui::DragFloat3("Scale", glm::value_ptr(scale));

		if (scale != node->GetLocalScale())
		{
			node->SetLocalScale(scale);
		}
	}


} // namespace lux
