// lowp is dramatically faster than medium or highp on the iPhone
precision lowp float;

uniform   sampler2D     uTexture;
uniform   vec4          uGlobalColor;

varying   vec4          vDiffuse;
varying   vec2          vTextureCoordinate0;

void main(void)
{
    float alpha = texture2D(uTexture, vTextureCoordinate0).a;

    gl_FragColor = vDiffuse * alpha;

//    gl_FragColor = vDiffuse;
//    gl_FragColor.a = min(alpha, vDiffuse.a);
}



