/*
 * Copyright (c) 2007-2011, Maxim Makhinya  <maxmah@gmail.com>
                 2013     , Ahmet Bilgili <ahmet.bilgili@epfl.ch>  Modified for single-pass raycasting
                 2014     , Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
 */

// input variables to function

#version 120
#extension GL_ARB_texture_rectangle : enable

#define EARLY_EXIT 0.99
#define DEFAULT_NSAMPLES 32

uniform sampler3D volumeTex; //gx, gy, gz, v
uniform sampler1D transferFnTex;
uniform sampler2DRect frameBufferTex;

uniform mat4 invProjectionMatrix;
uniform mat4 invModelViewMatrix;
uniform ivec4 viewport;

uniform vec3 aabbMin;
uniform vec3 aabbMax;
uniform vec3 textureMin;
uniform vec3 textureMax;
uniform vec3 voxelSpacePerWorldSpace;
uniform vec3 worldEyePosition;

uniform vec2 depthRange;
uniform ivec2 screenPos;

uniform int nSamples;
uniform float shininess;
uniform int refLevel;

struct Ray {
    vec3 Origin;
    vec3 Dir;
};

struct AABB {
    vec3 Min;
    vec3 Max;
};

// http://www.opengl.org/wiki/Compute_eye_space_from_window_space.
vec3 calcPositionInWorldSpaceFromWindowSpace( vec4 windowSpace )
{
    vec4 ndcPos;
    ndcPos.xy = 2.0 * ( windowSpace.xy - viewport.xy - ( viewport.zw / 2.0 ) ) / viewport.zw;
    ndcPos.z = 2.0 * ( windowSpace.z - ( ( depthRange.x + depthRange.y ) / 2.0 ) ) / ( depthRange.y - depthRange.x );
    ndcPos.w = 1.0;

    vec4 clipSpacePos = ndcPos / windowSpace.w;
    vec4 eyeSpacePos = invProjectionMatrix * clipSpacePos;
    eyeSpacePos = eyeSpacePos / eyeSpacePos.w;

    return vec3( ( invModelViewMatrix * eyeSpacePos ).xyz );
}

// Compute texture position.
vec3 calcTexturePositionFromAABBPos( vec3 pos )
{
    return ( pos - aabbMin ) / ( aabbMax - aabbMin ) * ( textureMax - textureMin ) + textureMin;
}

// AABB-Ray intersection ( http://prideout.net/blog/?p=64 ).
bool intersectBox( Ray r, AABB aabb, out float t0, out float t1 )
{
    vec3 invR = 1.0 / r.Dir;
    vec3 tbot = invR * ( aabb.Min-r.Origin);
    vec3 ttop = invR * ( aabb.Max-r.Origin);
    vec3 tmin = min( ttop, tbot );
    vec3 tmax = max( ttop, tbot );
    vec2 t = max( tmin.xx, tmin.yz );
    t0 = max( t.x, t.y );
    t = min( tmax.xx, tmax.yz );
    t1 = min( t.x, t.y );
    return t0 <= t1;
}

vec4 composite( vec4 src, vec4 dst, float alphaCorrection )
{
    float alpha = 1.0 - pow( ( 1.0 - src.a ), alphaCorrection );
    dst.rgb = dst.rgb + src.rgb * alpha * ( 1.0 - dst.a );
    dst.a += alpha * ( 1.0 - dst.a );
    return dst;
}

void main( void )
{
    vec4 result = texture2DRect( frameBufferTex, gl_FragCoord.xy - screenPos );
    if( result.a > EARLY_EXIT )
         discard;

    vec3 pixelWorldSpacePos = calcPositionInWorldSpaceFromWindowSpace( gl_FragCoord );
    vec3 rayDirection = normalize( pixelWorldSpacePos - worldEyePosition );
    Ray eye = Ray( worldEyePosition, rayDirection );

    AABB aabb = AABB( aabbMin, aabbMax );

    float tnear, tfar;
    intersectBox( eye, aabb, tnear, tfar );

    if(tnear < 0.0)
        tnear = 0.0;

    vec3 rayStart = eye.Origin + eye.Dir * tnear;
    vec3 rayStop = eye.Origin + eye.Dir * tfar;


    float stepSize = 1.0 / float( nSamples );
    float alphaCorrection = float( DEFAULT_NSAMPLES ) / float( nSamples );

    vec3 pos = rayStart;
    float travel = distance( rayStop, rayStart );
    vec3 step = normalize( rayStop - rayStart ) * stepSize;

    // Front-to-back absorption-emission integrator
    for ( int i = 0; travel > 0.0; ++i, pos += step, travel -= stepSize )
    {
        vec3 texPos = calcTexturePositionFromAABBPos( pos );
        float density = texture3D( volumeTex, texPos ).r;
        vec4 transferFn  = texture1D( transferFnTex, density );
        result = composite( transferFn, result, alphaCorrection );

        if( result.a > EARLY_EXIT )
            break;

        if( i > nSamples )
            break;

    }

    gl_FragColor = result;

}
