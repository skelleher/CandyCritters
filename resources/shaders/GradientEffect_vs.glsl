//precision highp float;

attribute highp vec4      aPosition;
attribute lowp  vec4      aDiffuse;
attribute lowp  vec2      aTextureCoordinate0;

uniform   highp mat4      uMatModelView;
uniform   highp mat4      uMatProjection;
uniform   lowp  vec4      uGlobalColor;

varying   lowp  vec2      vTextureCoordinate0;
varying   lowp  vec4      vDiffuse;


// Effect-specific parameters:
uniform   mediump  vec4   uStartColor;
uniform   mediump  vec4   uEndColor;
uniform   highp vec2      uStartPoint;
uniform   highp vec2      uEndPoint;
uniform   highp float     uWidth;
uniform   highp float     uHeight;

varying   mediump  vec4   vColor;


void main()
{
    // Transform the vertex into screen space
    gl_Position = uMatProjection * uMatModelView * vec4(aPosition.xyz, 1.0);

    // Compute the gradient color
    highp float distance1 = distance(aTextureCoordinate0, uStartPoint)/max(uWidth, uHeight);
    highp float distance2 = distance(aTextureCoordinate0, uEndPoint)/max(uWidth, uHeight);

//    lowp  vec4  gradient  = mix(uStartColor, uEndColor, distance2);

    lowp  vec4  gradient  = mix(uStartColor, uEndColor, distance1);

//    lowp  vec4  gradient;
//    gradient   =  (uStartColor * distance1);
//    gradient  +=  (uEndColor   * distance2);
//    normalize(gradient);

    // Pass to fragment shader
    vColor = uGlobalColor * aDiffuse * gradient;
    vTextureCoordinate0 = aTextureCoordinate0;
}

