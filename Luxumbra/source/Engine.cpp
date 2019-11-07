#include "Engine.h"

#include <set>

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_vulkan.h"

#include "glm\gtc\type_ptr.hpp"

namespace lux
{

	Engine::Engine() noexcept
		: isInitialized(false), window(), rhi(), scene(), resourceManager(rhi),
		currentTime(0.0f), previousTime(0.0f)
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

		scene.Initialize(window, rhi, resourceManager);

		isInitialized = true;


		return true;
	}

	void Engine::Run() noexcept
	{
		while (!window.ShouldClose())
		{
			DrawImgui();

			rhi.Render(scene.GetCurrentCamera(), scene.GetMeshNodes(), scene.GetLightNodes());

			window.PollEvents();
		}
	}

	scene::Scene& Engine::GetScene() noexcept
	{
		return scene;
	}

	resource::ResourceManager& Engine::GetResourceManager() noexcept
	{
		return resourceManager;
	}

	void Engine::DrawImgui() noexcept
	{
		//ImGuiStyle& style = ImGui::GetStyle();
		//style.Colors[ImGuiCol_WindowBg].w = 1.0f;

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		currentTime = glfwGetTime();
		double deltaTime = currentTime - previousTime;
		double frameDuration = deltaTime * 1000.0;
		double framerate = 1.0 / deltaTime;
		previousTime = currentTime;

		char buf[128];
		sprintf_s(buf, "Luxumbra Engine %d###Luxumbra Engine", (int)framerate);
	
		ImGui::Begin(buf);

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
					bool newShowToneMapping = showToneMapping;
					bool newShowFXAA = showFXAA;
					bool newShowSSAO = showSSAO;

					ImGui::Checkbox("Show Tone Mapping", &newShowToneMapping);
					ImGui::Checkbox("Show FXAA", &newShowFXAA);
					ImGui::Checkbox("Show SSAO", &newShowSSAO);

					int newSplitViewMask = 0;

					newSplitViewMask |= newShowToneMapping ? lux::rhi::PostProcessSplitViewMask::SPLIT_VIEW_TONE_MAPPING_MASK : 0;
					newSplitViewMask |= newShowFXAA ? lux::rhi::PostProcessSplitViewMask::SPLIT_VIEW_FXAA_MASK : 0;
					newSplitViewMask |= newShowSSAO ? lux::rhi::PostProcessSplitViewMask::SPLIT_VIEW_SSAO_MASK : 0;

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

			if (ImGui::BeginTabItem("Scene"))
			{
				scene::CameraNode* camera = scene.GetCurrentCamera();
				DisplayCameraNode(camera);
				ImGui::NewLine();

				DisplayMeshNodes(scene.GetMeshNodes());
				ImGui::NewLine();
				DisplayLightNodes(scene.GetLightNodes());
				ImGui::NewLine();

				DisplayMaterials();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}


		ImGui::End();

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

	void Engine::DisplayMaterials() noexcept
	{
		if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_DefaultOpen))
		{
			std::set<std::string> materialNames;
			const std::vector<scene::MeshNode*>& meshNodes = scene.GetMeshNodes();
			for (size_t i = 0; i < meshNodes.size(); i++)
			{
				resource::Material* currentMaterial = &meshNodes[i]->GetMaterial();
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
