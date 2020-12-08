### Stéphane Pontadit & Matthieu de Laharpe

# LuxUmbra -  Light & Shadow Engine

LuxUmbra is a light & shadow engine that runs with the Vulkan API.<br>
The goals of the project were to learn the **Vulkan API** and modern rendering techniques like **Physically Based-Rendering** (PBR) and **Post-Process** effects. We did not focus on performance and optimization.

It was developed in one month by Matthieu de Laharpe and Stephane Pontadit.<br>

![Overview](README_Resources/Models.jpg)

## Table of Contents

+ [How to launch](#How-to-Launch)
+ [Controls](#Controls)
+ [Editor](#Editor)
+ [Shaders Hot Reload](#Shaders-Hot-Reload)
+ [Technical Document](#Technical-Document)
  + [Illumination Model](#Illumination-Model)
  + [Shadows](#shadows)
  + [Image Based-Lighting](#Image-Based-Lighting)
  + [Post Process & Anti-Aliasing](#Post-Proscess-&-Anti-Aliasing)
  + [References](#References)

<br>

## **How to Launch**

+ ### Run Executable

> Extract "LuxUmbra_Binary.rar" archive and run "LuxUmbra.exe"

+ ### Compile the project

> Open "LuxUmbra.sln" solution file. You may need to re-target the project.<br>Compile and run the project.

<br>

## **Controls**

+ W : forward
+ S : backward
+ A : left
+ D : right
+ E : up
+ C/Alt : down
+ I : show/hide editor
+ Echap : exit

<br>

## **Editor**

The editor is built with Omar Cornut's "dear imgui" library.

+ **Change Scenes:** Change the current scene by clicking on the radio butons at the top of the editor.
+ **Post-Process:**
  + *Debug:* Show tone mapping, FXAA, SSAO or direct color only on the left part of the screen. The "percent" slider controls the horizontal position of the white line that splits the screen.
  + *Tone Mapping:* Exposes the tone mapping algorithm and the exposure factor.
  + *FXAA:* Exposes the overall FXAA quality.
  + *SSAO:* Exposes SSAO settings.
+ **Render Settings:** Exposes shadow mapping parameters.
+ **Scenes:**  Exposes camera, meshes, lights and materials properties.

![Editor](README_Resources/Editor.png)

<br>

## **Shaders Hot Reload**

1. Make changes to shaders
2. Run LuxUmbra/data/shaders/Compilers.bat
3. Click on the "Reload Shader" button in the editor

<br>

## **Technical Document**

### **Illumination Model**

All the calculations for illumination are done in **"cameraSpaceLight.frag"** shader.

#### **Diffuse**

We implemented two different BRDFs for the diffuse term. We have the **Lambertian BRDF** that assumes a uniform diffuse response, and we have **Disney's diffuse BRDF** (by Burley) that takes the roughness into account to create reflections at grazing angles. In the engine, we use Disney's BRDF by default.

#### **Specular**

For the specular part, we implemented the **Cook-Torrance BRDF**. The different terms of the BRDF that we chose are:

+ **GGX (Trowbridge-Reitz)** for the Normal Distribution Function
+ **Schlick Approximation** for the Fresnel Equation
+ **Smith GGX** for the Masking-Shadowing Function

<ins>**Figure 1:**</ins> different roughness, reflectance and metalness values

![PBR Sphere](README_Resources/PBR_Spheres.jpg)

<br>

#### **Clear Coat**

We implemented a simplified version of material layering with a **Cleat Coat**. It is calculated with the same Cook-Torrance BRDF but with different terms:

+ **GGX** for the Normal Distributon Function
+ **Schlick Approximation** for the Fresnel Equation
+ **Kelement** for the Masking-Shadowing Function

<ins>**Figure 2:**</ins> different cleat coat roughness and intensity values

![ClearCoat](README_Resources/Clear_Coat.jpg)

<br>

#### **Material**

5 different maps are handled: Albedo, Normal, Roughness, Metallic and Ambient Occlusion that allow us to have complex materials.<br>

<ins>**Figure 3:**</ins> a collection of complex physically based materials

![Materials](README_Resources/Materials.jpg)

<ins>**Figure 4:**</ins> marble material with and without clear coat

![MaterialClearCoat](README_Resources/Marble.gif)

<br>

### **Shadows**

We implemented shadow-mapping for directional and point lights. <br>

For directional shadow mapping, we create an AABB in the coordinate's frame of the light whose dimension define an orthographic projection matrix. The scene depth is rendered using this projection to create the shadow map. During the scene render, for each light, we transform the position of a fragment in the coordinate's frame of the light and compare its distance to the light with the depth sampled in the shadow map using the transformed position. <br>

Shadows from directional light sources are anti-aliased using the **Percentage Closer Filtering** (PCF) algorithm. <br>

<ins>**Figure 5:**</ins> model is illuminated by two directional lights that produce shadows on the walls

![Direction Shadow](README_Resources/Shadow_Directional.jpg)

<br>

For point lights, we create the shadow map by rendering the depth of the scene from the position of the light into a cubemap. Then, when we render the scene, just like for directional lights, we transform the position of a fragment in the coordinate's frame of each light and compare the distance to the light with the depth sampled in the shadow map using the direction from the light to the transformed fragment position.

<br><br><br>

### **Image Based-Lighting**

Our environment maps are computed from HDR textures and we use them to do **Image-Based Lighting**. <br>

#### **Diffuse Irradiance**

The diffuse part of IBL is achieved by computing the irradiance of the environment map by convolution. This result is stored in a cubemap. This is our **irradiance map**.

#### **Specular**

Specular part is calculated with the split sum approximation made famous by Epic Games. The first sum is pre-calculated for different roughness values and the results are stored in the mip-map levels of a cubemap. This is our **prefiltered map**. The second sum is a 2D LUT that represents the BRDF's response given an input roughness and an input angle between the normal and the light direction. <br>

Objet that use IBL reflections sample the prefiltered map using their roughness value to access a mip level of the map. Higher mip levels are blurrier than lower mip levels, so rough material have blurred reflections and smooth material have sharp reflections.<br>

These two maps depend on the environment map, we generate them at program startup with a **compute shader**<br>

<ins>**Figure 6:**</ins> the environment is reflected on the surface of the helmet

![IBL](README_Resources/Helmet_Light.jpg)

<br>

### **Post-Proscess & Anti-Aliasing**

All the calculation for post-processing are done in **"blit.frag"** shader.

#### **Anti-Aliasing**

We implemented two anti-aliasing techniques. The first one is the **Multisampling Anti-Aliasing** (MSAA) on the Vulkan side. It uses up to four samples per pixel. We also implemented **Fast Approximate Anti-Aliasing** (FXAA) as a post-process to smooth edges.

#### **Post-Process**

We implemented several **Tone Mapping algorithms**: Reinhard, ACES Film and Uncharted2. The tone mapping remaps HDC color to LDC color. All our color calculations are done in linear color space. We apply gamma correction during the post-process stage.<br>
The last post-process that we implemented is **Screen-Space Ambient Occlusion** (SSAO). We use a hybrid pipeline with traditional forward rendering plus we keep buffers with the data necessary to SSAO computation. We use a normal and a position buffer to calculate ambient occlusion on a fragment and apply this occlusion during the post-process pass.<br>

<ins>**Figure 7:**</ins> same image with and without SSAO

![SSAO](README_Resources/SSAO.gif)

<br>

### **References**

#### *Rendering*

+ [Real shading in Unreal Engine 4](https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf) by Brian Karis, Epic Games, Siggraph 2013 presentation note
+ [Moving Frostbite to Physically Based Rendering 3.0](https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf) by Sébastien Lagarde and Charles de Rousiers, Electronic Arts, Siggraph 2014
+ [Real-time Physicaly Based Rendering](https://en.ppt-online.org/378584) by Yoshiharu Gotanda and Tatsuya Shoji, R&D Repartment, tri-Ace, Inc.
+ [Enterprise PBR Shading Model](https://dassaultsystemes-technology.github.io/EnterprisePBRShadingModel/user_guide.md.html) by Dassault Systèmes for general references about PBR and shading
+ [Filament project](https://google.github.io/filament/Filament.md.html) by Google for general references about PBR and shading
+ [Filament Crafting Physicaly-Based Materials](https://google.github.io/filament/Material%20Properties.pdf) for material references
+ [Brian Karis's blog](https://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html) for Specular BRDF Reference
+ [A Simple and Practical Approach to SSAO](https://www.gamedev.net/articles/programming/graphics/a-simple-and-practical-approach-to-ssao-r2753/) for an explanation of the SSAO algorithm
+ [Daniel-Alin Loghin's blog](http://alinloghin.com/articles/compute_ibl.html) for IBL resources calculation
+ [Simon Rodriguez's blog](http://blog.simonrodriguez.fr/articles/30-07-2016_implementing_fxaa.html) for an explanation of the FXAA algorithm

### *Vulkan*

+ [Sascha Willems GitHub repository](https://github.com/SaschaWillems/Vulkan) for general references about Vulkan API
+ [NVIDIA Vulkan Shader Resource Binding blog post](https://developer.nvidia.com/vulkan-shader-resource-binding) for optimizing resource bindings by updated them based on usage frequency
