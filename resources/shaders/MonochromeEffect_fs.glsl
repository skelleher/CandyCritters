// lowp is dramatically faster than medium or highp on the iPhone
precision lowp float;

uniform   sampler2D     uTexture;
uniform   bool          uTextureEnabled;
uniform   bool          uLightingEnabled;
uniform   vec4          uGlobalColor;

varying   vec4          vDiffuse;
varying   vec2          vTextureCoordinate0;


void main(void)
{
    vec4 color = texture2D( uTexture, vTextureCoordinate0 ) * vDiffuse;
        
    // Convert to grayscale
//    float gray       = dot(vec3(0.22, 0.71, 0.07), color.rgb);
    float gray       = dot(vec3(0.3, 0.59, 0.11), color.rgb);
    gl_FragColor.rgb = vec3(gray, gray, gray);
    gl_FragColor.a   = color.a;
}
