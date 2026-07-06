#ifndef WIND_INCLUDED
#define WIND_INCLUDED

float Wind_hash(vec2 p) 
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float Wind_noise(vec2 p) 
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = Wind_hash(i);
    float b = Wind_hash(i + vec2(1.0, 0.0));
    float c = Wind_hash(i + vec2(0.0, 1.0));
    float d = Wind_hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x)
         + (c - a) * u.y * (1.0 - u.x)
         + (d - b) * u.x * u.y;
}

float Wind_GetBendAmout(float height)
{
    return height * height;
}

vec3 Wind_CalculateWind(vec3 windDir, vec3 positionWS, float height, float windStrength, float time)
{
    float bend = Wind_GetBendAmout(height);

    float noise = Wind_noise(positionWS.xz * 0.15);

    float t = time * 2.0 + noise * 6.2831;

    float sway = sin(t) * 0.45 + sin(t * 0.5) * 0.3;

    // float windNoiseSample = Wind_noise(positionWS.xz) * 0.5 + 1 * time;
    // windDir = vec3(sin(windNoiseSample), 0, cos(windNoiseSample));

    vec3 wind = windDir * (sway * windStrength * bend);

    return wind;
}

// vec3 Wind_CalculateNormal(vec3 normalWS, float height)
// {
//     vec3 wind = vec3(WindDir.x, 0.0, WindDir.z);

//     vec3 normal = normalize(mix(normalWS, wind, Wind_GetBendAmout(height)));

//     return normal;
// }

#endif //WIND_INCLUDED