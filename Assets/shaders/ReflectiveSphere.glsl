#shader vertex

#include "VertexDataLayout.glsl"
#include "Core.glsl"

out vec3 v_posWS;
out vec2 v_uv;
out vec3 v_normal;

void main()
{
   vec4 posWS = Core_ObjectToWorldPosition(VA_POSITION);
   gl_Position = Core_WorldToClipSpace(posWS);
   v_uv = VA_UV;
   v_normal = Core_TransformNormal(VA_NORMAL);
   v_posWS = posWS.xyz;
}

#shader fragment

#include "Core.glsl"

in vec2 v_uv;
in vec3 v_posWS;
in vec3 v_normal;

out vec4 FragColor;

void main()
{
    vec3 result = Core_CalculateLighting(v_normal, v_posWS) * vec3(1, 1, 1);

    result = Core_MixFogWS(v_posWS, result);
   
    FragColor = vec4(result, 1);
}