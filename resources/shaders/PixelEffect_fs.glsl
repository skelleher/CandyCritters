precision mediump float;


uniform   sampler2D     uTexture;
uniform   bool          uTextureEnabled;
uniform   bool          uLightingEnabled;
uniform   vec4          uGlobalColor;

uniform   float         uNumVerticalSamples;
uniform   float         uNumHorizontalSamples;
uniform   float         uVerticalSampleSize;
uniform   float         uHorizontalSampleSize;

varying   vec4          vDiffuse;
varying   vec2          vTextureCoordinate0;


vec2 downsampleCoordinate( vec2 coordinate, float numHorizontalSamples, float numVerticalSamples, float horizontalSampleSize, float verticalSampleSize )
{
    vec2 downsampledCoordinate;

    // Which sample bucket does this pixel fall in?
    float xbucket = floor(coordinate.x  / horizontalSampleSize);
    float ybucket = floor(coordinate.y  / verticalSampleSize);

    //clamp(downsampledCoordinate.x = xbucket / numHorizontalSamples, 0.0, 1.0);
    //clamp(downsampledCoordinate.y = ybucket / numVerticalSamples, 0.0, 1.0);

    downsampledCoordinate.x = xbucket / numHorizontalSamples;
    downsampledCoordinate.y = ybucket / numVerticalSamples;

    return downsampledCoordinate;
}


void main(void)
{
    vec2 coord = downsampleCoordinate( vTextureCoordinate0, uNumHorizontalSamples, uNumVerticalSamples, uHorizontalSampleSize, uVerticalSampleSize );
/*
    vec4 tint;

    if (coord.y <= 0.5 && coord.x <= 0.5)
    {
        tint = vec4(1.0, 0.0, 0.0, 1.0);
    }
    else if (coord.y <= 0.5 && coord.x >= 0.5)
    {
        tint = vec4(0.0, 1.0, 0.0, 1.0);
    }
    else if (coord.y >= 0.5 && coord.x <= 0.5)
    {
        tint = vec4(0.0, 0.0, 1.0, 1.0);
    }
    else if (coord.y >= 0.5 && coord.x >= 0.5)
    {
        tint = vec4(1.0, 0.0, 1.0, 1.0);
    }
    else
    {
        tint = vec4(1.0, 1.0, 1.0, 1.0);
    }

    gl_FragColor = tint * texture2D( uTexture, coord );
*/

    gl_FragColor = vDiffuse * texture2D( uTexture, coord );
}


