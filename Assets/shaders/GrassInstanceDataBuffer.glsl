#ifndef INS_DATA_BUFFER_INCLUDED
#define INS_DATA_BUFFER_INCLUDED
layout(std430, binding = 10) buffer InstanceDataBuffer
{
    mat4 PerInstanceModelMatrices[];
};
#endif