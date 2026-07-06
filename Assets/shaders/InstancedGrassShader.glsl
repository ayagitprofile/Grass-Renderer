#shader vertex

#include "VertexDataLayout.glsl"
#include "Core.glsl"
#include "Wind.glsl"

#include "GrassInstanceDataBuffer.glsl"

layout(std430, binding = 12) buffer CullingDataBuffer
{
    uint VisibleChunks;
    uint InstanceDataOffsets[]; // Size = number of chunks, contains offsets to the start of a chunk data
};

#include "SharedStateBuffer.glsl"

out vec3 v_normal;
out vec3 v_posWS;
out vec2 v_uv;

uniform float u_time;
uniform vec4 u_wind = vec4(normalize(vec3(0.5, 0, -0.5)), 0.5);

float hash11(float p)
{
    p = fract(p * 0.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float hash12(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

void main()
{
    const uint NumberOfChunks = InstanceDataOffsets.length();

    const uint ChunkIndex = gl_InstanceID / SharedState.BladesPerChunk;
    const uint ChunkLocalIndex = gl_InstanceID % SharedState.BladesPerChunk;
    const uint ActualDataBegin = InstanceDataOffsets[ChunkIndex];
    const uint ActualInstanceData = ActualDataBegin + ChunkLocalIndex;

    const mat4 instanceModelMatrix = PerInstanceModelMatrices[ActualInstanceData];

    const float WindAmplitude = 0.2;
    const float BendAmplitude = 0.2;
    float perInstanceHashNoise = hash11(ActualInstanceData);

    vec3 bendLS = vec3(0, 0, (VA_UV.y * VA_UV.y) * BendAmplitude * perInstanceHashNoise);

    vec3 posOS = VA_POSITION + bendLS;
    
    posOS += mix(-0.3, 0.1, perInstanceHashNoise) * VA_UV.y;

    vec4 posWS = instanceModelMatrix * vec4(posOS, 1.0);

    float faloff = 1.0 - smoothstep(75, 115, distance(posWS.xyz, CORE_CAMERA_POSITION));

    posWS = vec4(posWS.xyz + Wind_CalculateWind(u_wind.xyz, posWS.xyz, VA_UV.y, u_wind.w, u_time) * faloff, posWS.w);
    
    v_normal = Core_TransformNormal(VA_NORMAL, instanceModelMatrix);
    
    v_uv = VA_UV;
    v_posWS = posWS.xyz;
    gl_Position = Core_WorldToClipSpace(posWS);
}

#shader fragment

#include "Core.glsl"

in vec3 v_normal;
in vec3 v_posWS;
in vec2 v_uv;

out vec4 FragColor;

uniform float u_time;

const vec3 TopColor = vec3(0.761, 0.839, 0.051);
const vec3 BotColor = vec3(0.216, 0.412, 0.031);

void main()
{
    vec3 normal1 = mix(-v_normal, v_normal, float(float(gl_FrontFacing) > 0.5));
    vec3 normal2 = normalize(cross(dFdx(v_posWS), dFdy(v_posWS)));

    vec3 normal = mix(normal1, normal2, 0.5);        

    vec3 color = mix(BotColor, TopColor, pow(v_uv.y, 3));

    float ao = Core_Saturate(pow(v_uv.y, 1.5) * 3);

    vec3 result = Core_CalculateLighting(normal, v_posWS) * ao * color;

    result = Core_MixFogWS(v_posWS, result);

    FragColor = vec4(result, 1);
    // FragColor = vec4(ao.xxx, 1);
}