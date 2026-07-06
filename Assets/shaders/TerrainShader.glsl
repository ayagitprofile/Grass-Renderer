#shader vertex

#include "VertexDataLayout.glsl"
#include "Core.glsl"

out vec2 v_uv;
out vec3 v_posWS;
out vec3 v_normal;
out mat3 v_TBN;

uniform sampler2D u_normalMap;
uniform sampler2D u_heightMap;

#include "TerrainData.glsl"

void main()
{
    // v_TBN = Core_TBNMatrix(VA_NORMAL, VA_TANGENT, VA_TANGENT_HANDEDNESS);

    // float height = texture(u_heightMap, VA_UV).x;
    // float normal = texture(u_normalMap, VA_UV);

    float h = texture(u_heightMap, VA_UV).x * TERRAIN_HEIGHT_SCALE;

    vec2 texelSize = 1.0 / textureSize(u_heightMap, 0);
    float hL = texture(u_heightMap, VA_UV - vec2(texelSize.x, 0)).r * TERRAIN_HEIGHT_SCALE;
    float hR = texture(u_heightMap, VA_UV + vec2(texelSize.x, 0)).r * TERRAIN_HEIGHT_SCALE;
    float hD = texture(u_heightMap, VA_UV - vec2(0, texelSize.y)).r * TERRAIN_HEIGHT_SCALE;
    float hU = texture(u_heightMap, VA_UV + vec2(0, texelSize.y)).r * TERRAIN_HEIGHT_SCALE;

    float dx = hR - hL;
    float dz = hU - hD;

    float cellSize = 100 / texelSize.x;

    vec3 tangentX = vec3(2.0 * cellSize, dx, 0.0);
    vec3 tangentZ = vec3(0.0, dz, 2.0 * cellSize);

    vec3 normal = normalize(cross(tangentZ, tangentX));

    vec4 posWS = Core_ObjectToWorldPosition(VA_POSITION + vec3(0, h, 0));
    
    vec3 T = normalize(vec3(2.0 * cellSize, dx, 0.0));
    vec3 B = normalize(vec3(0.0, dz, 2.0 * cellSize));
    vec3 N = normalize(cross(B, T));

    // Re-orthogonalize to ensure a proper basis
    T = normalize(T - N * dot(T, N));
    B = cross(N, T);

    mat3 TBN = mat3(T, B, N);

    v_uv = VA_UV;
    v_posWS = posWS.xyz;
    v_normal = normal;
    v_TBN = TBN;
    
    gl_Position = Core_WorldToClipSpace(posWS);
}

#shader fragment

#include "Core.glsl"
#include "TerrainData.glsl"

uniform sampler2D u_colorTexture;
uniform sampler2D u_normalMap;
uniform sampler2D u_heightMap;

uniform sampler2D u_sandColorMap;
uniform sampler2D u_sandNormalMap;

in vec2 v_uv;
in vec3 v_posWS;
in vec3 v_normal;
in mat3 v_TBN;

out vec4 FragColor;

void main()
{
    vec2 tiledUV = fract(v_uv * 30);
    vec2 sandTiledUV = fract(v_uv * 40);

    float blendWidth = TERRAIN_BLEND;
    float edge = TERRAIN_SAND_LEVEL;
    float surfaceBlend = smoothstep(edge - blendWidth, edge + blendWidth, texture(u_heightMap, v_uv)).x;

    vec3 normal = mix(Core_SampleNormalMap(u_sandNormalMap, v_TBN, sandTiledUV), Core_SampleNormalMap(u_normalMap, v_TBN, tiledUV), surfaceBlend);

    vec4 color = mix(texture(u_sandColorMap, sandTiledUV), texture(u_colorTexture, tiledUV), surfaceBlend);

    vec3 objectColor = Core_CalculateLighting(normal, v_posWS) * (mix(1.0, 0.5, surfaceBlend)) * color.xyz;

    vec3 result = Core_MixFogWS(v_posWS, objectColor);

    FragColor = vec4(result, 1);
}