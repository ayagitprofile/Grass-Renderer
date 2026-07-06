#ifndef VERTEX_DATA_LAYOUT_DEFINED
#define VERTEX_DATA_LAYOUT_DEFINED

#define VERTEX_LAYOUT_P_N_UV_TAN

#ifdef VERTEX_SHADER
#ifdef VERTEX_LAYOUT_P_N_UV
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aUV;
#define VA_POSITION (aPos)
#define VA_NORMAL (aNorm)
#define VA_UV (aUV)
#define VA_EMPTY_ATTRIB_SLOT (3)
#endif //#ifdef VERTEX_LAYOUT_P_N_UV

#ifdef VERTEX_LAYOUT_P_N_UV_TAN
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec4 aTangent;
#define VA_POSITION (aPos)
#define VA_NORMAL (aNorm)
#define VA_UV (aUV)
#define VA_TANGENT (aTangent.xyz)
#define VA_TANGENT_HANDEDNESS (aTangent.w)
#define VA_EMPTY_ATTRIB_SLOT (4)
#endif //VERTEX_LAYOUT_P_N_UV_TAN

#endif //#ifdef VERTEX_SHADER

#endif //#ifndef VERTEX_DATA_LAYOUT_DEFINED
