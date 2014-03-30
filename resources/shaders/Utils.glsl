
//
// Pseudo-random number generator.
//
float rand(vec2 v)
{
    return fract(sin(dot(v.xy ,vec2(12.9898,78.233))) * 43758.5453);
}


//
// Checkerboard pattern.
// 
const      int          frequency = 10;
const      vec4         color0    = vec4(1.0, 1.0, 1.0, 0.5);
const      vec4         color1    = vec4(0.0, 0.0, 0.0, 0.5);
vec4 checkerBoard(void)
{
    vec2  texcoord = mod(floor(vTextureCoordinate0 * float(frequency*2)), 2.0);
    float delta    = abs(texcoord.x - texcoord.y);
    return mix(color1, color0, delta);
}

