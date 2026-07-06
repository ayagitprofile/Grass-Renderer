#shader vertex

#include "VertexDataLayout.glsl"
#include "Core.glsl"

out vec3 vPos;

void main()
{
    // vec4 positionCS = CORE_CAMERA_PROJECTION_MATRIX * mat4(mat3(CORE_CAMERA_VIEW_MATRIX)) * vec4(VA_POSITION, 1.0);
    // gl_Position = positionCS.xyww;
    // vPos = VA_POSITION;

    mat4 viewNoTranslation = mat4(mat3(CORE_CAMERA_VIEW_MATRIX));

    vec4 pos = CORE_CAMERA_PROJECTION_MATRIX * viewNoTranslation * vec4(VA_POSITION, 1.0);

    // Force depth to the far plane
    gl_Position = pos.xyww;

    // Pass direction to fragment shader
    vPos = VA_POSITION;
}

#shader fragment

#include "Core.glsl"

in vec3 vPos;
out vec4 FragColor;

void main()
{
    // vec3 dir = normalize(vPos);

    // float height = clamp(dir.y, 0, 1);

    // vec3 topColor = vec3(0.02, 0.749, 0.8);
    // vec3 botColor = (0.5).xxx;
    
    // vec3 result = botColor * (1.0 - height) + topColor * pow(height, 0.7f);

    // float sun = clamp(pow(dot(-dir, CORE_DIR_LIGHT_DIRECTION), 1000.0), 0, 1);
    // vec3 sunColor = sun * CORE_DIR_LIGHT_COLOR;

    // FragColor = vec4(Core_SampleSkybox(vPos + CORE_CAMERA_POSITION) * (1 - sun) + sunColor, 1);
    FragColor = vec4(Core_SampleSkybox(vPos + CORE_CAMERA_POSITION), 1);
}