precision highp float;

attribute vec4      aPosition;
attribute vec4      aDiffuse;
attribute vec2      aTextureCoordinate0;

uniform   mat4      uMatModelView;
uniform   mat4      uMatProjection;

varying   vec2      vTextureCoordinate0;
varying   vec4      vDiffuse;
varying   vec2      vCoord;


/*
void myRefract( in vec3 incom, 
                in vec3 normal, 
                in float index_external, 
                in float index_internal, 
                out vec3 reflection, 
                out vec3 refraction, 
                out float reflectance, 
                out float transmittance ) 
{ 
    float eta = index_external/index_internal; 
    float cos_theta1 = dot(incom, normal); 
    float cos_theta2 = sqrt(1.0 - ((eta * eta) * ( 1.0 - (cos_theta1 * cos_theta1)))); 
    reflection = incom - 2.0 * cos_theta1 * normal; 
    refraction = (eta * incom) + (cos_theta2 - eta * cos_theta1) * normal; 
    float fresnel_rs = (index_external * cos_theta1 - index_internal * cos_theta2 ) / (index_external * cos_theta1 + index_internal * cos_theta2); 
    float fresnel_rp = (index_internal * cos_theta1 - index_external * cos_theta2 ) / (index_internal * cos_theta1 + index_external * cos_theta2); 
    reflectance = (fresnel_rs * fresnel_rs + fresnel_rp * fresnel_rp) / 2.0; transmittance =((1.0-fresnel_rs) * (1.0-fresnel_rs) + (1.0-fresnel_rp) * (1.0-fresnel_rp)) / 2.0; 
} 
*/


void main()
{
    // Transform the vertex into screen space
    gl_Position = uMatProjection * uMatModelView * vec4(aPosition.xyz, 1.0);

    // Pass to fragment shader
    vDiffuse              = aDiffuse;
    vTextureCoordinate0   = aTextureCoordinate0;
    
/*
    vec2 vPosition        = normalize(gl_Position.xy);
    vCoord                = vPosition.xy/2.0 + 0.5;
    vCoord                = vCoord + 0.1*tan(vCoord*5.0);
*/

/*
    vec3 reflection, refraction;
    float reflectance, transmittance;
    myRefract( vec3(0,0,-1), vec3(0,0,1), 0.1, 0.1, reflection, refraction, reflectance, transmittance );
    
    vCoord = aTextureCoordinate0 + refraction.xy;
*/
}

