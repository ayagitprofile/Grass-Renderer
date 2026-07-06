#ifndef TERRAIN_DATA_INCLUDED
#define TERRAIN_DATA_INCLUDED

#define TERRAIN_HEIGHT_SCALE (15.0)
#define TERRAIN_SAND_LEVEL (0.0)
// #define TERRAIN_SAND_LEVEL (0.23)
#define TERRAIN_BLEND (0.05)

float Terrain_smoothstep(float x){
    const float blendWidth = TERRAIN_BLEND;
    const float edge = TERRAIN_SAND_LEVEL;
    return smoothstep(edge - blendWidth, edge + blendWidth, x);
}

float Terrain_smoothstep(float x, float blend){
    const float blendWidth = blend;
    const float edge = TERRAIN_SAND_LEVEL;
    return smoothstep(edge - blendWidth, edge + blendWidth, x);
}


#endif // #ifndef TERRAIN_DATA_INCLUDED