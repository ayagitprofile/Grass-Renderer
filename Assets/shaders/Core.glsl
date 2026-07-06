#ifndef CAMERA_DATA_INCLUDED
#define CAMERA_DATA_INCLUDED

// For correct allignment only using vec2 (8 bytes) or vec4(16 bytes) because vec3(12 bytes) is padded to 16 bytes 

#ifndef FRUSTUM_PLANE_INCLUDED
#define FRUSTUM_PLANE_INCLUDED
struct FrustumPlane
{
	vec3 normal;
	float d;
};
#endif

layout(std140, binding = 0) uniform Core_Camera
{
    mat4 g_view;
    mat4 g_projection;
    vec4 g_cameraPos;
    vec4 g_CameraForward;
    mat4 g_vp;
    FrustumPlane g_cameraPlanes[6];
};

layout(std140, binding = 1) uniform Core_LightingData
{
    vec4 g_DirectionalLightDirection;
    vec4 g_DirectionalLightColor;
    vec4 g_AmbientLight;
    vec4 g_FogRange;
};
#define CORE_CAMERA_FRUSTUM_PLANES (g_cameraPlanes)
#define CORE_CAMERA_VIEW_MATRIX (g_view)
#define CORE_CAMERA_PROJECTION_MATRIX (g_projection)
#define CORE_CAMERA_POSITION (g_cameraPos.xyz)
#define CORE_CAMERA_VP_MATRIX (g_vp)
// #define CORE_CAMERA_VP_MATRIX (CORE_CAMERA_PROJECTION_MATRIX * CORE_CAMERA_VIEW_MATRIX)
#define CORE_CAMERA_FORWARD (g_CameraForward.xyz)
#define CORE_TIME (g_FogRange.z)

#define CORE_DIR_LIGHT_DIRECTION (g_DirectionalLightDirection.xyz)
#define CORE_DIR_LIGHT_COLOR (g_DirectionalLightColor.xyz)
#define CORE_AMBIENT_LIGHT (g_AmbientLight.xyz)

float Core_Saturate(float x)
{
    return clamp(x, 0.0, 1.0);
}

float Core_FogFactor(vec3 positionWS)
{
    float distance = length(positionWS - CORE_CAMERA_POSITION);
    float fog = clamp((distance - g_FogRange.x) / (g_FogRange.y - g_FogRange.x), 0.0, 1.0);
    return fog;
}

const vec3 Core_SkyboxTopColor = vec3(0, 0.31, 0.6);
const vec3 Core_SkyboxBotColor = (0.6).xxx;

// const vec3 Core_SkyboxTopColor = vec3(0.02, 0.749, 0.8);

// Sample pure skybox color without sun
vec3 Core_SampleSkyboxColor(float height)
{
    return mix(Core_SkyboxBotColor, Core_SkyboxTopColor, Core_Saturate((pow(height, 0.65) + 0.3)));
}

// For ambient light
vec3 Core_SampleSkyboxAmbient(vec3 normalWS)
{
    vec3 dir = normalWS;
    float height = clamp(dir.y, 0, 1);
    vec3 skyboxColor = Core_SampleSkyboxColor(height);
    return skyboxColor * length(CORE_AMBIENT_LIGHT);
}

vec3 Core_SampleSkybox(vec3 positionWS)
{
    vec3 dir = normalize(positionWS - CORE_CAMERA_POSITION);
    float height = clamp(dir.y, 0, 1);
    vec3 skyboxColor = Core_SampleSkyboxColor(height);
    float sun = clamp(pow(dot(-dir, CORE_DIR_LIGHT_DIRECTION), 1000.0) * 1.5, 0, 1);
    vec3 sunColor = sun * CORE_DIR_LIGHT_COLOR;
    return skyboxColor * (1 - sun) + sunColor;
}

vec3 Core_MixFogWS(vec3 positionWS, vec3 objectColor)
{
    float distance = length(positionWS - CORE_CAMERA_POSITION);
    float fogFactor = clamp((distance - g_FogRange.x) / (g_FogRange.y - g_FogRange.x), 0.0, 1.0);

    vec3 dir = normalize(positionWS - CORE_CAMERA_POSITION);
    float height = clamp(dir.y, 0, 1);

    vec3 skyboxColor = Core_SampleSkyboxColor(height);

    float sun = clamp(pow(dot(-dir, CORE_DIR_LIGHT_DIRECTION), 1000.0), 0, 1) * fogFactor;
    vec3 sunColor = sun * CORE_DIR_LIGHT_COLOR;
    vec3 skyboxSample = skyboxColor * (1 - sun) + sunColor;

    return mix(objectColor, skyboxSample, fogFactor);
}

#ifdef VERTEX_SHADER

uniform mat4 u_model;
#define CORE_MODEL_MATRIX (u_model)

vec3 Core_TransformNormal(vec3 normalOS)
{
    // return normalize(CORE_NORMAL_MATRIX * normalOS);
    return normalize(transpose(inverse(mat3(CORE_MODEL_MATRIX))) * normalOS);
}

vec3 Core_TransformNormal(vec3 normalOS, mat4 modelMatrix)
{
    // return normalize(CORE_NORMAL_MATRIX * normalOS);
    return normalize(transpose(inverse(mat3(modelMatrix))) * normalOS);
}

vec4 Core_ObjectToWorldPosition(vec3 positionOS)
{
    return CORE_MODEL_MATRIX * vec4(positionOS, 1.0f);
}

vec4 Core_WorldToClipSpace(vec4 positionWS)
{
    return CORE_CAMERA_VP_MATRIX * positionWS;
}

vec4 Core_WorldToViewSpace(vec4 positionWS)
{
    return CORE_CAMERA_VIEW_MATRIX * positionWS;
}

vec4 Core_ViewToClipSpace(vec4 positionVS)
{
    return CORE_CAMERA_PROJECTION_MATRIX * positionVS;
}

mat3 Core_TBNMatrix(vec3 normal, vec3 tangent, float handedness)
{
    vec3 N = Core_TransformNormal(normal);
    vec3 T = normalize(mat3(CORE_MODEL_MATRIX) * tangent);
    // Make tangent orthogonal to normal (important!)
    T = normalize(T - dot(T, N) * N);
    // Reconstruct bitangent using handedness
    vec3 B = cross(N, T) * handedness;
    return mat3(T, B, N);
}

#endif // #ifdef VERTEX_SHADER

#ifdef FRAGMENT_SHADER

vec3 Core_SampleNormalMap(sampler2D normalMap, mat3 TBN, vec2 uv)
{
    return normalize(TBN * (texture(normalMap, uv).rgb * 2.0 - 1.0));
}

uniform float u_specPow = 32;
uniform float u_specStr = 0.5;

vec3 Core_CalculateLighting(vec3 normal)
{
    // Diffuse
    float diffuse = max(dot(normal, -CORE_DIR_LIGHT_DIRECTION), 0.0);
    vec3 diffuseColor = CORE_DIR_LIGHT_COLOR * diffuse;
    return (CORE_AMBIENT_LIGHT + diffuseColor);
}

vec3 Core_CalculateSpecular(vec3 normal, vec3 positionWS)
{
    // Diffuse
    float diffuse = max(dot(normal, -CORE_DIR_LIGHT_DIRECTION), 0.0);
    vec3 diffuseColor = CORE_DIR_LIGHT_COLOR * diffuse;
    
    // Specular
    vec3 viewDir = normalize(CORE_CAMERA_POSITION - positionWS);
    vec3 reflectDir = reflect(CORE_DIR_LIGHT_DIRECTION, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_specPow);
    vec3 specularColor = u_specStr * spec * CORE_DIR_LIGHT_COLOR;

    return specularColor;
}

vec3 Core_CalculateLighting(vec3 normal, vec3 positionWS)
{
    // Diffuse
    float diffuse = max(dot(normal, -CORE_DIR_LIGHT_DIRECTION), 0.0);
    vec3 diffuseColor = CORE_DIR_LIGHT_COLOR * diffuse;
    
    // Specular
    vec3 viewDir = normalize(CORE_CAMERA_POSITION - positionWS);
    vec3 reflectDir = reflect(CORE_DIR_LIGHT_DIRECTION, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_specPow);
    vec3 specularColor = u_specStr * spec * CORE_DIR_LIGHT_COLOR;

    vec3 cameraToSurface = -viewDir;
    vec3 surfaceToSkybox = reflect(cameraToSurface, normal);
    vec3 ambient = mix(Core_SampleSkyboxAmbient(normal), Core_SampleSkyboxAmbient(surfaceToSkybox), 0.5);

    // return (CORE_AMBIENT_LIGHT + diffuseColor + specularColor);
    return (ambient + diffuseColor + specularColor);
}

#endif //#ifdef FRAGMENT_SHADER

#endif //#ifndef CAMERA_DATA_INCLUDED