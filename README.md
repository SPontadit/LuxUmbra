# LuxUmbra -  Light & Shadow Engine

LuxUmbra is a light & shadow engine that runs with Vulkan API.<br>
The project's goals were to learn Vulkan API and modern rendering techniques like PBR.

It was develop in one month in a group of two people. <br><br><br>

## Table of Contents

+ [Controles](#Controles)
+ [Illumination Model](#Illumination-Model)
+ [Shadows](#shadows)
+ [Image Based-Lighting](#Image-Based-Lighting)
+ [Post Process & Anti-Aliasing](#Post-Proscess-&-Anti-Aliasing) 
+ [Shaders Hot Reload](#Shaders-Hot-Reload)
+ [Possible Issue](#Possible-Issue)
<br><br><br>

## **Controles**

- W : forward
- S : backward
- A : left
- D : right
- E : up
- C/Alt : down
- I : show/hide editor
- Echap : exit <br><br><br>

## **Illumination Model**

ee <br><br><br>

## **Shadows**

Work In Progress <br><br><br>

## **Image Based-Lighting**

Environment map are from HDR texture and we use it to do **image-based lighing**. <br>
Ambient Diffuse is done with an **irradience map** (heavily blurred version of environment map). <br>
Specular is done with a **prefiltered map**. <br><br>
Objet that use IBL reflection sample the prefiltered map using its roughness value to access mip level of the map. Higher mip levels are blurier than lower mip levels. So rough material have blured reflection and smooth material have sharp reflection.<br><br>
This two maps depending on the environment map, we compute then at program startup with **compute shader** <br><br><br>

## **Post Proscess & Anti-Aliasing**

### **Anti-Aliasing**

We have two anti-alliasing technique. The first one is the Multisampling Anti-Aliasing (MSAA) on the vulkan side. We are up to four samples per pixels. We also have Fast Approximate Anti-Aliasing (FXAA) as a post process to smooth edge.<br><br>

### **Post-Process**

We have several tone mapping algorythm, Reinhard, ACES Film and Uncharted2. The tone mapping is to remap HDC color to LDC color. All our color calculations are done in linear color space. We do gamma correction on post-process.<br>
The last but not least post-process is Screen-Space Ambient Occlusion (SSAO). We have a part of deferred rendering. We use normal and position buffer to calculate ambient occlusion of the fragment.<br><br><br>

## **Shaders Hot Reload**

1. Makes change to shaders
2. Run LuxUmbra/data/shaders/Compilers.bat
3. Click on "Reload Shader" button on editor <br><br><br>

## **Possible Issue**

If you do not use Visual Studio 2019 or you do not use Platform Toolset "Visual Studio 2019(v142), you must go to project properties and change Platform Toolset to your current platform toolset.

## Screen

![Platform Toolset](image:https://carbon-media.accelerator.net/0000000jAgd/k0fAQQIN2mMeZn9V5fOk5V;1920x1080.jpeg?auto=webp)
