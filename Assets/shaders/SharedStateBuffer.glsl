#ifndef SHARED_STATE_BUFFER_INCLUDED
#define SHARED_STATE_BUFFER_INCLUDED

layout(std430, binding = 14) buffer SharedStateBuffer
{
	uint BladesPerChunk;
    float GrassRenderingDistance;
} SharedState;

#endif //#ifndef SHARED_STATE_BUFFER_INCLUDED