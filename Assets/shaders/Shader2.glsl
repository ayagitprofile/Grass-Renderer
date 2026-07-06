#shader vertex

#include "VertexDataLayout.glsl"
#include "Core.glsl"

out vec3 v_posWS;
out vec2 v_uv;
out mat3 v_TBN;

void main()
{
   vec4 posWS = Core_ObjectToWorldPosition(VA_POSITION);
   gl_Position = Core_WorldToClipSpace(posWS);
   v_uv = VA_UV;
   v_posWS = posWS.xyz;
   v_TBN = Core_TBNMatrix(VA_NORMAL, VA_TANGENT, VA_TANGENT_HANDEDNESS);
}

#shader fragment

#include "Core.glsl"

in vec2 v_uv;
in vec3 v_posWS;
in mat3 v_TBN;

out vec4 FragColor;

uniform sampler2D u_colorTexture;
uniform sampler2D u_normalMap;

void main()
{
    vec3 objectColor = texture(u_colorTexture, v_uv).rgb;
    
    vec3 normal = Core_SampleNormalMap(u_normalMap, v_TBN, v_uv);

    vec3 result = Core_CalculateLighting(normal, v_posWS) * objectColor;

    result = Core_MixFogWS(v_posWS, result);
   
    FragColor = vec4(result, 1);
}