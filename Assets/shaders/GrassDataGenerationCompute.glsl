#shader compute

#include "ComputeData.glsl"
#include "TerrainData.glsl"

#include "GrassInstanceDataBuffer.glsl"

layout(std430, binding = BOUNDING_BOX_DATA_BINDING) buffer OutputAABBDataBuffer
{
    AABB PerChunkAABBs[];
} OutputAABBData;

#include "SharedStateBuffer.glsl"

vec3 AABBMax(vec3 left, vec3 right)
{
	return vec3
	(
		max(left.x, right.x),
		max(left.y, right.y),
		max(left.z, right.z)
	);
}

vec3 AABBMin(vec3 left, vec3 right)
{
	return vec3
	(
		min(left.x, right.x),
		min(left.y, right.y),
		min(left.z, right.z)
    );
}

mat4 Translate(vec3 t)
{
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        t.x, t.y, t.z, 1.0
    );
}

mat4 RotateY(float angle) {
    float c = cos(angle);
    float s = sin(angle);

    return mat4(
         c, 0.0, -s, 0.0,
         0.0, 1.0,  0.0, 0.0,
         s, 0.0,  c, 0.0,
         0.0, 0.0, 0.0, 1.0
    );
}

mat4 ScaleMatrix(vec3 s)
{
    return mat4(
        s.x, 0.0, 0.0, 0.0,
        0.0, s.y, 0.0, 0.0,
        0.0, 0.0, s.z, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

float Hash12(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

uniform sampler2D TerrainHeightTexture;

void main()
{
    const uvec2 ChunkID = gl_WorkGroupID.xy;
    const uvec2 BladeID = gl_LocalInvocationID.xy;
    
    const float BigNumber = 1000000;
    AABB aabb;
    aabb.minpoint = vec4(BigNumber, BigNumber, BigNumber, 0);
    aabb.maxpoint = vec4(-BigNumber, -BigNumber, -BigNumber, 0);

    const uint BladesPerRow = BLADES_PER_ROW;
    const uint ChunksPerRow = CHUNKS_PER_ROW;
    // const uint BladesPerChunk = BLADES_PER_CHUNK;
    const uint ChunkDataIndex = gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x;
    const vec2 TerrainBegin = TERRAIN_BEGIN;
    const vec2 TerrainEnd = TERRAIN_END;
    const vec2 TerrainArea = TERRAIN_AREA;
    const vec3 TerrainCenter = TERRAIN_CENTER;
    const float bladesPerGlobalRow = BladesPerRow * ChunksPerRow;
    const float MaxRandomOffset = MAX_RANDOM_OFFSET; 

    for (uint y = 0; y < BladesPerRow; ++y)
    {
        for (uint x = 0; x < BladesPerRow; ++x)
        {
            const uint localIndex = x + y * BladesPerRow;
            const uint chunkIndex = ChunkID.x + ChunkID.y * ChunksPerRow;

            const uint dataIndex = localIndex + chunkIndex * SharedState.BladesPerChunk;

            const uint globalx = ChunkID.x * BladesPerRow + x;
            const uint globaly = ChunkID.y * BladesPerRow + y;

            const float px = mix(TerrainBegin.x, TerrainEnd.x, float(globalx) / (bladesPerGlobalRow - 1));
            const float pz = mix(TerrainBegin.y, TerrainEnd.y, 1.0 - float(globaly) / (bladesPerGlobalRow - 1));

            vec3 position = vec3(px, 0, pz);

            float hashNoise = Hash12(position.xz);
            float hashNoise2 = Hash12(vec2(position.x, -position.z));

            position += vec3(MaxRandomOffset * hashNoise, 0, MaxRandomOffset * hashNoise2);

            const vec2 offset = vec2(position.x, position.z) - TerrainBegin;
            const vec2 uv = offset / TerrainArea;

            const float height = texture(TerrainHeightTexture, vec2(uv.x, 1 - uv.y)).x;
            const float scaledHeight = height * TERRAIN_HEIGHT_SCALE;

            const float combinedHash = mix(hashNoise, hashNoise2, 0.5);

            const float randomAngle = mix(0.0, radians(350.0), combinedHash);

            const float randomScale = mix(0.9, 1.1, combinedHash);

            position.y = scaledHeight;

            aabb.maxpoint = vec4(AABBMax(aabb.maxpoint.xyz, position), 0);
            aabb.minpoint = vec4(AABBMin(aabb.minpoint.xyz, position), 0);

            // OutputMatrixData.PerInstanceModelMatrices[dataIndex] = Translate(position) * RotateY(randomAngle);
            mat4 modelMatrix = Translate(position) * RotateY(randomAngle) * ScaleMatrix((Terrain_smoothstep(height - 0.05 + sin(combinedHash) * 0.05, 0.0)).xxx);
            PerInstanceModelMatrices[dataIndex] = modelMatrix;
        }
    }

    // aabb.maxpoint.y += 10;
    // aabb.minpoint.y -= 10;

    // float temp = aabb.minpoint.z;
    // aabb.minpoint.z = aabb.maxpoint.z;
    // aabb.maxpoint.z = temp;

    OutputAABBData.PerChunkAABBs[ChunkDataIndex] = aabb;
}