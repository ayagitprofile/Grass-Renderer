#ifndef COMPUTE_DATA_INCLUDED
#define COMPUTE_DATA_INCLUDED

#ifndef FRUSTUM_PLANE_INCLUDED
#define FRUSTUM_PLANE_INCLUDED
struct FrustumPlane
{
	vec3 normal;
	float d;
};
#endif

struct CameraPlanes
{
    FrustumPlane left;
    FrustumPlane right;
    FrustumPlane bottom;
    FrustumPlane top;
    FrustumPlane near;
    FrustumPlane far;
};

struct AABB {
	vec4 minpoint;
	vec4 maxpoint;
};

#endif