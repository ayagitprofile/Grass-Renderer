#shader vertex

#include "VertexDataLayout.glsl"
#include "Core.glsl"

out vec3 v_posWS;
out vec2 v_uv;
out mat3 v_TBN;
uniform sampler2D u_normalMap;

const float UVScale = 1.5;

void main()
{
    vec4 posWS = Core_ObjectToWorldPosition(VA_POSITION);
    gl_Position = Core_WorldToClipSpace(posWS);
    v_uv = VA_UV;

    vec2 tiledUV = VA_UV * UVScale;
    vec3 normalSample1 = texture(u_normalMap, tiledUV + vec2(-CORE_TIME * 0.01, 0)).xyz;
    vec3 normalSample2 = texture(u_normalMap, tiledUV + vec2(0, CORE_TIME * 0.01)).xyz;
    vec3 normal = mix(normalSample1, normalSample2, 0.5);

    v_posWS = posWS.xyz + normal * 0.5;
    v_TBN = Core_TBNMatrix(VA_NORMAL, VA_TANGENT, VA_TANGENT_HANDEDNESS);
}

#shader fragment

#include "Core.glsl"

in vec2 v_uv;
in vec3 v_posWS;
in mat3 v_TBN;

out vec4 FragColor;

uniform sampler2D u_colorMap;
uniform sampler2D u_normalMap;

const float UVScale = 1.5;

void main()
{
    vec2 tiledUV = v_uv * UVScale;

    // FragColor = vec4(0, 0.1, 0.9, 1);
    vec3 objectColor = vec3(0.904, 1.0, 1.0) * 0.7;
    // vec3 objectColor = texture(u_colorMap, v_uv).rgb;

    const float timeScale = 0.015;

    vec3 normalSample1 = Core_SampleNormalMap(u_normalMap, v_TBN, tiledUV + vec2(-CORE_TIME * timeScale, 0));
    vec3 normalSample2 = Core_SampleNormalMap(u_normalMap, v_TBN, tiledUV + vec2(0, CORE_TIME * timeScale));
    vec3 normal = mix(normalSample1, normalSample2, 0.5);
    // vec3 normal = normalize(normalSample1 + normalSample2);

    vec3 result = Core_CalculateLighting(normal, v_posWS) * objectColor;

    result = Core_MixFogWS(v_posWS, result);

    float transparency = clamp(1 - max(dot(normalize(CORE_CAMERA_POSITION - v_posWS), normal), 0.0) + 0.1, 0.2, 1.0);
   
    FragColor = vec4(result, transparency);
}