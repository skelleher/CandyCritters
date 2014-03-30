uniform   sampler2D     uTexture;
uniform   mediump vec4  uGlobalColor;
//uniform   bool          uTextureEnabled;
//uniform   bool          uLightingEnabled;
uniform   bool          uHorizontalPass;
uniform   mediump float uWidth;
uniform   mediump float uHeight;

varying   mediump  vec4 vDiffuse;
varying   highp    vec2 vTextureCoordinate0;

// Convolution kernel for Gaussian Blur, up to 7x1
const     int           MAX_FILTER_SIZE   = 7;
uniform   mediump float  uFilterOffsets[MAX_FILTER_SIZE];
uniform   mediump float  uFilterWeights[MAX_FILTER_SIZE];



//
// Cheap, fast Neighborhood blur.  
// Does not look as smooth as a Convolution (Gaussian) blur.
//
const highp float offset = 0.005;
mediump vec4 SimpleBlur( sampler2D texture, highp vec2 texcoord, highp float offset )
{
    mediump vec4 sampleN = vec4(0.0);
    mediump vec4 sampleS = vec4(0.0);
    mediump vec4 sampleE = vec4(0.0);
    mediump vec4 sampleW = vec4(0.0);

    sampleN = texture2D( texture, vec2(texcoord.x,           texcoord.y - offset) );
    sampleS = texture2D( texture, vec2(texcoord.x,           texcoord.y + offset) );
    sampleE = texture2D( texture, vec2(texcoord.x - offset,  texcoord.y) );
    sampleW = texture2D( texture, vec2(texcoord.x + offset,  texcoord.y) );
    
    return (sampleN + sampleS + sampleE + sampleW)*0.25;
}



//
// Convolution (Gaussian) blur.
//
mediump vec4 ConvolutionBlur( sampler2D texture, mediump vec2 texcoord, mediump float filterOffsets[MAX_FILTER_SIZE], mediump float filterWeights[MAX_FILTER_SIZE] )
{
    mediump vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

    for (int index = 0; index < MAX_FILTER_SIZE; ++index)
    {
        mediump vec2 coord;
        if (uHorizontalPass)
        {
            coord = vec2(texcoord.x + filterOffsets[index], texcoord.y);
        }
        else
        {
            coord = vec2(texcoord.x, texcoord.y + filterOffsets[index]);
        }
        
        mediump vec4 temp = texture2D( texture, coord );
        color += (temp * filterWeights[index]);
    }
    
    return color;
}


void main(void)
{
//    gl_FragColor = vDiffuse * ConvolutionBlur( uTexture, vTextureCoordinate0, uFilterOffsets, uFilterWeights );
    gl_FragColor = vDiffuse * SimpleBlur( uTexture, vTextureCoordinate0, offset );
}
