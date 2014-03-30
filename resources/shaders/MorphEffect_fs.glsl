precision mediump float;

uniform   sampler2D     uTexture;
uniform   bool          uTextureEnabled;
uniform   bool          uLightingEnabled;
uniform   vec4          uGlobalColor;

varying   vec4          vDiffuse;
varying   vec2          vTextureCoordinate0;

varying   vec3          vLeft;
varying   vec3          vRight;
varying   vec3          vMiddle;


uniform    bool         uCheckerboard;
const      int          frequency = 10;
const      vec4         color0    = vec4(1.0, 1.0, 1.0, 0.5);
const      vec4         color1    = vec4(0.0, 0.0, 0.0, 0.5);
vec4 checkerBoard(void)
{
    vec2  texcoord = mod(floor(vTextureCoordinate0 * float(frequency*2)), 2.0);
    float delta    = abs(texcoord.x - texcoord.y);
    return mix(color1, color0, delta);
}



void main(void)
{
    // Don't show the backside of a sprite; useful for "twist" animations.
    if (!gl_FrontFacing)
    {
        discard;
    }


    vec4 color = texture2D(uTexture, vTextureCoordinate0) * vDiffuse;

    // Uncomment for checker-board pattern
    //gl_FragColor = mix(color, checkerBoard(), 0.25);
    gl_FragColor = color;

    // DEBUG: draw the bezier control curves
//    if (length(gl_FragCoord.xy - vLeft.xy) < 4.0)
//    {
//        gl_FragColor = vec4(0, 1, 0, 1);
//    }
//
//    if (length(gl_FragCoord.xy - vRight.xy) < 4.0)
//    {
//        gl_FragColor = vec4(1, 0, 0, 1);
//    }
//
//    if (length(gl_FragCoord.xy - vMiddle.xy) < 4.0)
//    {
//        gl_FragColor = vec4(0, 0, 1, 1);
//    }


    // DEBUG: draw the bounding box
//    if (vTextureCoordinate0.x < 0.01 || vTextureCoordinate0.x > 0.99 ||
//        vTextureCoordinate0.y < 0.01 || vTextureCoordinate0.y > 0.99   )
//    {
//        gl_FragColor = vec4(1, 0, 0, 1);
//    }
}
