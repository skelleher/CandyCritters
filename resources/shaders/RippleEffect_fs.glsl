precision lowp float;

uniform   sampler2D uTexture;
uniform   vec4      uGlobalColor;

varying   vec2      vTextureCoordinate0;
varying   vec4      vDiffuse;

varying   vec4      vWaveColor;

void main(void)
{
//    gl_FragColor = vDiffuse * texture2D(uTexture, vTextureCoordinate0) + vWaveColor;
    gl_FragColor = vDiffuse * texture2D(uTexture, vTextureCoordinate0);
}