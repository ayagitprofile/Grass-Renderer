#pragma once

namespace Grass { namespace Const {
	inline constexpr const char* MeshesPath = "Assets/Meshes/";
	inline constexpr const char* ShadersPath = "Assets/shaders/";
	inline constexpr const char* TexturesPath = "Assets/textures/";

	inline constexpr const char* OpenGLVersionString = "460";
	inline constexpr const char* OpenGLVersionDirective = "#version 460";
	
	inline constexpr const char* AlbedoMapName = "u_albedoMap";
	inline constexpr const char* NormalMapName = "u_normalMap";

	inline constexpr auto CameraUBOBindingIndex = 0;
	inline constexpr auto LightDataUBOBindingIndex = CameraUBOBindingIndex + 1;
	inline constexpr auto STD14LayoutVec3Size = 16;
} 
}