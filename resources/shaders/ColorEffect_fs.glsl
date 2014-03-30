// lowp is dramatically faster than medium or highp on the iPhone
precision lowp float;

uniform   sampler2D     uTexture;
uniform   bool          uTextureEnabled;
uniform   bool          uLightingEnabled;
uniform   vec4          uColor;
uniform   vec4          uGlobalColor;

varying   vec4          vDiffuse;
varying   vec2          vTextureCoordinate0;
varying   float         vBlend;


void main(void)
{
    vec4 source = texture2D( uTexture, vTextureCoordinate0 ) * vDiffuse;

    // TODO: trust that this was done on the CPU?
    vec4 color = normalize(uColor);

    // Convert to target color space
    float temp       = dot(uColor.rgb, source.rgb);

    gl_FragColor.rgb = mix(source.rgb, vec3(temp, temp, temp), vBlend);
    gl_FragColor.a   = min(source.a, uColor.a);
}
