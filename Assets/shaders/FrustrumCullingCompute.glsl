#shader compute

#include "FrustumPlane.glsl"
#include "ComputeData.glsl"
#include "Core.glsl"

float DistanceToPlane(FrustumPlane plane, vec3 point)
{
	return dot(plane.normal, point) + plane.d;
}

CameraPlanes CalculateFrustumPlanes(mat4 clip)
{
    CameraPlanes planes;
	// Left
	planes.left.normal = vec3(
		clip[0][3] + clip[0][0],
		clip[1][3] + clip[1][0],
		clip[2][3] + clip[2][0]
	);
	planes.left.d = clip[3][3] + clip[3][0];
	
	// Right
	planes.right.normal = vec3(
		clip[0][3] - clip[0][0],
		clip[1][3] - clip[1][0],
		clip[2][3] - clip[2][0]
	);
	planes.right.d = clip[3][3] - clip[3][0];
	
	// Bottom
	planes.bottom.normal = vec3(
		clip[0][3] + clip[0][1],
		clip[1][3] + clip[1][1],
		clip[2][3] + clip[2][1]
	);
	planes.bottom.d = clip[3][3] + clip[3][1];
	
	// Top
	planes.top.normal = vec3(
		clip[0][3] - clip[0][1],
		clip[1][3] - clip[1][1],
		clip[2][3] - clip[2][1]
	);
	planes.top.d = clip[3][3] - clip[3][1];
	
	// Near
	planes.near.normal = vec3(
		clip[0][3] + clip[0][2],
		clip[1][3] + clip[1][2],
		clip[2][3] + clip[2][2]
	);
	planes.near.d = clip[3][3] + clip[3][2];
	
	// Far
	planes.far.normal = vec3(
		clip[0][3] - clip[0][2],
		clip[1][3] - clip[1][2],
		clip[2][3] - clip[2][2]
	);
	planes.far.d = clip[3][3] - clip[3][2];
	
	// Normalize planes

    float len = length(planes.left.normal);
	planes.left.normal /= len;
	planes.left.d /= len;

    len = length(planes.right.normal);
	planes.right.normal /= len;
	planes.right.d /= len;    

    len = length(planes.top.normal);
	planes.top.normal /= len;
	planes.top.d /= len;    

    len = length(planes.bottom.normal);
	planes.bottom.normal /= len;
	planes.bottom.d /= len;

    len = length(planes.near.normal);
	planes.near.normal /= len;
	planes.near.d /= len;

    len = length(planes.far.normal);
	planes.far.normal /= len;
	planes.far.d /= len;    

	return planes;
}

#if 1

#define TEST_PLANE(PLANE) \
{ \
    vec3 n = (PLANE).normal; \
    vec3 pv = mix(aabb.maxpoint.xyz, aabb.minpoint.xyz, lessThan(n, vec3(0.0))); \
    if (dot(n, pv) + (PLANE).d < 0.0) return false; \
}

bool IntersectFrustrumAABB(AABB aabb, CameraPlanes planes)
{
    TEST_PLANE(planes.left);
    TEST_PLANE(planes.right);
    TEST_PLANE(planes.top);
    TEST_PLANE(planes.bottom);
    TEST_PLANE(planes.near);
    TEST_PLANE(planes.far);

    return true;
}

#else

bool IntersectFrustrumAABB(AABB aabb, CameraPlanes planes) {

	// Select "positive vertex" (farthest in direction of plane normal)
    FrustumPlane plane = planes.left;

	vec3 positiveVertex = vec3(
		(plane.normal.x >= 0) ? aabb.maxpoint.x : aabb.minpoint.x,
		(plane.normal.y >= 0) ? aabb.maxpoint.y : aabb.minpoint.y,
		(plane.normal.z >= 0) ? aabb.maxpoint.z : aabb.minpoint.z
    );
	// If positive vertex is outside, box is completely outside
	if (DistanceToPlane(plane, positiveVertex) < 0)
		return false;

    plane = planes.right;

	positiveVertex = vec3(
		(plane.normal.x >= 0) ? aabb.maxpoint.x : aabb.minpoint.x,
		(plane.normal.y >= 0) ? aabb.maxpoint.y : aabb.minpoint.y,
		(plane.normal.z >= 0) ? aabb.maxpoint.z : aabb.minpoint.z
	);
	// If positive vertex is outside, box is completely outside
	if (DistanceToPlane(plane, positiveVertex) < 0)
		return false;

    plane = planes.top;

	positiveVertex = vec3(
		(plane.normal.x >= 0) ? aabb.maxpoint.x : aabb.minpoint.x,
		(plane.normal.y >= 0) ? aabb.maxpoint.y : aabb.minpoint.y,
		(plane.normal.z >= 0) ? aabb.maxpoint.z : aabb.minpoint.z
	);
	// If positive vertex is outside, box is completely outside
	if (DistanceToPlane(plane, positiveVertex) < 0)
		return false;

    plane = planes.bottom;
	positiveVertex = vec3(
		(plane.normal.x >= 0) ? aabb.maxpoint.x : aabb.minpoint.x,
		(plane.normal.y >= 0) ? aabb.maxpoint.y : aabb.minpoint.y,
		(plane.normal.z >= 0) ? aabb.maxpoint.z : aabb.minpoint.z
	);
	// If positive vertex is outside, box is completely outside
	if (DistanceToPlane(plane, positiveVertex) < 0)
		return false;

    plane = planes.near;

	positiveVertex = vec3(
		(plane.normal.x >= 0) ? aabb.maxpoint.x : aabb.minpoint.x,
		(plane.normal.y >= 0) ? aabb.maxpoint.y : aabb.minpoint.y,
		(plane.normal.z >= 0) ? aabb.maxpoint.z : aabb.minpoint.z
    );
	// If positive vertex is outside, box is completely outside
	if (DistanceToPlane(plane, positiveVertex) < 0)
		return false;

    plane = planes.far;

	positiveVertex = vec3(
		(plane.normal.x >= 0) ? aabb.maxpoint.x : aabb.minpoint.x,
		(plane.normal.y >= 0) ? aabb.maxpoint.y : aabb.minpoint.y,
		(plane.normal.z >= 0) ? aabb.maxpoint.z : aabb.minpoint.z
	);
	// If positive vertex is outside, box is completely outside
	if (DistanceToPlane(plane, positiveVertex) < 0)
		return false;

	return true; // intersects or fully inside
}

#endif

struct DrawElementsIndirectCommand
{
	uint Count;         // Number of indices
	uint InstanceCount; // Number of instances
	uint FirstIndex;    // Starting index
	int BaseVertex;     // Added to each index
	uint BaseInstance;  // Base instance ID
};

#include "GrassInstanceDataBuffer.glsl"

layout(std430, binding = INDIRECT_BUFFER_BINDING) buffer IndirectDrawBuffer
{
	DrawElementsIndirectCommand Command;
} CommandBuffer;

layout(std430, binding = VISIBILITY_DATA_BINDING) buffer CullingDataBuffer
{
	uint VisibleChunks;
    uint InstanceDataOffsets[]; // Size = number of chunks, contains offsets to the start of a chunk data
};

#include "SharedStateBuffer.glsl"

layout(std430, binding = BOUNDING_BOX_DATA_BINDING) buffer AABBDataBuffer
{
    AABB PerChunkAABBs[];
} AABBData;

void main()
{
	const uint dataIndex = gl_LocalInvocationIndex; // flat chunk id

#ifdef CLEAR_COMPUTE_CULLING_DATA
	if (dataIndex == 0){
		CommandBuffer.Command.InstanceCount = 0;
		VisibleChunks = 0;
	}
	barrier();
#endif

	AABB aabb = AABBData.PerChunkAABBs[dataIndex];
#if 0
	vec3 center = (aabb.minpoint.xyz + aabb.maxpoint.xyz) * 0.5f;
	vec3 extents = (aabb.maxpoint.xyz - aabb.minpoint.xyz) * 0.5f;
	float radius = length(extents);

	vec3 toChunk = center - CORE_CAMERA_POSITION;
	float forwardDist = dot(toChunk, -CORE_CAMERA_FORWARD);

	// Entire chunk is behind the camera
	if (forwardDist < -radius)
		return;
#endif

	FrustumPlane[] cameraPlanes = CORE_CAMERA_FRUSTUM_PLANES;
	CameraPlanes planes;
	
	planes.left   = cameraPlanes[0];
	planes.right  = cameraPlanes[1];
	planes.top    = cameraPlanes[2];
	planes.bottom = cameraPlanes[3];
	planes.near   = cameraPlanes[4];
	planes.far    = cameraPlanes[5];

	if (distance(aabb.maxpoint.xyz, CORE_CAMERA_POSITION) < SharedState.GrassRenderingDistance && IntersectFrustrumAABB(aabb, planes)){
		const uint currentChunk = atomicAdd(VisibleChunks, 1);

		InstanceDataOffsets[currentChunk] = dataIndex * SharedState.BladesPerChunk;
		atomicAdd(CommandBuffer.Command.InstanceCount, SharedState.BladesPerChunk);
	}
}