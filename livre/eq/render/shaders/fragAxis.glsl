#version 420

#define AXIS 1

layout( rgba32f ) uniform image2DRect renderTexture;

uniform mat4 invProjectionMatrix;

flat in vec3 normal;
flat in vec3 normal2;
flat in vec3 eyePos;
in vec4 color;
flat in uint type;

void main()
{
    vec4 dst = imageLoad( renderTexture, ivec2( gl_FragCoord.xy ));

    const float dotNormal = dot( eyePos, normal );
    const float dotNormal2 = dot( eyePos, normal2 );

    if( type == AXIS )
    {
        if(( dotNormal > 0.0 ) && ( dotNormal2 < 0.0 ) || 
           ( dotNormal < 0.0 ) && ( dotNormal2 > 0.0 ))    
        {
            if( abs( dot( eyePos, normal )) > abs( dot( eyePos, normal2 )))
                imageStore( renderTexture, ivec2( gl_FragCoord.xy ), vec4( color.rgb , 1.0 ));
        }
    }
    else
    {
        if( dot( eyePos, normal ) <= 0.0 )
        {
            dst.rgb += color.rgb * color.a * ( 1.0 - dst.a );
            dst.a += color.a * ( 1.0 - dst.a );
            imageStore( renderTexture, ivec2( gl_FragCoord.xy ), dst );
        }
    }
}
