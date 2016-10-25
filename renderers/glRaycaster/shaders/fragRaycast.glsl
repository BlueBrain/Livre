/*
 * Copyright (c) 2007-2011, Maxim Makhinya  <maxmah@gmail.com>
                 2013     , Ahmet Bilgili <ahmet.bilgili@epfl.ch>  Modified for single-pass raycasting
                 2014     , Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
 */

// input variables to function
#version 420 core
#extension GL_ARB_texture_rectangle : enable

#define EARLY_EXIT 0.999
#define EPSILON 0.0000000001f
#define SH_UINT 0
#define SH_INT 1
#define SH_FLOAT 2

uniform uint datatype;
uniform sampler3D volumeTexFloat;
uniform usampler3D volumeTexUint;
uniform isampler3D volumeTexInt;

uniform vec2 dataSourceRange;

uniform sampler1D transferFnTex;
layout( location = 0 ) out vec4 FragColor;
layout( rgba32f ) uniform image2DRect renderTexture;

uniform mat4 invProjectionMatrix;
uniform mat4 invModelViewMatrix;
uniform ivec4 viewport;
uniform float nearPlaneDist;

uniform vec3 globalAABBMin;
uniform vec3 globalAABBMax;
uniform vec3 aabbMin;
uniform vec3 aabbMax;
uniform vec3 textureMin;
uniform vec3 textureMax;
uniform vec3 worldEyePosition;

uniform int nSamplesPerRay;
uniform int maxSamplesPerRay;
uniform int nSamplesPerPixel;
uniform int nClipPlanes;
uniform vec4 clipPlanes[6];

struct Ray
{
    vec3 Origin;
    vec3 Dir;
};

struct AABB
{
    vec3 Min;
    vec3 Max;
};

float rand(vec2 co)
{
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// http://www.opengl.org/wiki/Compute_eye_space_from_window_space.
vec4 calcPositionInEyeSpaceFromWindowSpace( vec4 windowSpace )
{
    vec4 ndcPos = vec4( 1.0f );
    ndcPos.xy = 2.0 * ( windowSpace.xy - viewport.xy - ( viewport.zw / 2.0 ) ) / viewport.zw;
    vec4 eyeSpacePos = invProjectionMatrix * ndcPos;
    return eyeSpacePos / eyeSpacePos.w;
}

// Compute texture position.
vec3 calcTexturePositionFromAABBPos( vec3 pos )
{
    return ( pos - aabbMin ) / ( aabbMax - aabbMin ) * ( textureMax - textureMin ) + textureMin;
}

// AABB-Ray intersection ( http://prideout.net/blog/?p=64 ).
bool intersectBox( Ray r, AABB aabb, out float t0, out float t1 )
{
    //We need to avoid division by zero in "vec3 invR = 1.0 / r.Dir;"
    if( r.Dir.x == 0 )
        r.Dir.x = EPSILON;

    if( r.Dir.y == 0 )
        r.Dir.y = EPSILON;

    if( r.Dir.z == 0 )
        r.Dir.z = EPSILON;

    vec3 invR = 1.0 / r.Dir;
    vec3 tbot = invR * ( aabb.Min - r.Origin );
    vec3 ttop = invR * ( aabb.Max - r.Origin );
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
    // The alpha correction function behaves badly around maximum alpha
    float alpha = 1.0 - pow( 1.0 - min( src.a, 1.0 - 1.0 / 256.0), alphaCorrection );
    dst.rgb += src.rgb * alpha * ( 1.0 - dst.a );
    dst.a += alpha * ( 1.0 - dst.a );
    return dst;
}

void main( void )
{
    vec4 result = imageLoad( renderTexture, ivec2( gl_FragCoord.xy ));
    if( result.a > EARLY_EXIT )
        discard;

    vec4 brickResult = vec4( 0.0f );

    for( int i = 0; i < nSamplesPerPixel; i++ )
    {
        float xPixelDelta = rand( vec2( gl_FragCoord.x * i, gl_FragCoord.y * i )) / 2.0f;
        float yPixelDelta = rand( vec2( gl_FragCoord.x * 2 * i , gl_FragCoord.y * 2 * i )) / 2.0f;
        vec4 localResult = result;

        vec4 subPixelCoord = gl_FragCoord + vec4( xPixelDelta, yPixelDelta, 0.0f, 0.0f );
        vec4 pixelEyeSpacePos = calcPositionInEyeSpaceFromWindowSpace( subPixelCoord );
        vec3 pixelWorldSpacePos = vec3(( invModelViewMatrix * pixelEyeSpacePos ).xyz );

        vec3 rayDirection = normalize( pixelWorldSpacePos - worldEyePosition );
        Ray eye = Ray( worldEyePosition, rayDirection );

        AABB aabb = AABB( aabbMin, aabbMax );
        AABB globalAABB = AABB( globalAABBMin, globalAABBMax );

        float tnearGlobal, tfarGlobal;
        if( !intersectBox( eye, globalAABB, tnearGlobal, tfarGlobal ))
            discard;

        float tnear, tfar;
        if( !intersectBox( eye, aabb, tnear, tfar ))
            discard;

        vec3 nearPlaneNormal = vec3( 0.0f, 0.0f, 1.0f );
        float tNearPlane = dot( nearPlaneNormal, vec3( 0.0, 0.0, -nearPlaneDist ))
                           / dot( nearPlaneNormal, normalize( pixelEyeSpacePos.xyz ));

        if( tnear < tNearPlane )
            tnear = tNearPlane;

        float stepSize = 1.0 / float( nSamplesPerRay );

        float residu = mod( tnear - tnearGlobal, stepSize );

        if( residu > 0.0f )
            tnear += stepSize - residu;

        if( tnear > tfar )
            discard;

        for( int i = 0; i < nClipPlanes; i++ )
        {
            vec3 planeNormal = clipPlanes[ i ].xyz;
            float rn = dot( rayDirection, planeNormal );
            if( rn == 0 )
                rn = EPSILON;
            float d = clipPlanes[ i ].w;
            float t = -( dot( planeNormal, worldEyePosition ) + d ) / rn;
            if( rn > 0.0 ) // opposite direction plane
                tnear = max( tnear, t );
            else
                tfar = min( tfar, t );
        }

        if( tnear > tfar )
            discard;

        vec3 rayStart = eye.Origin + eye.Dir * tnear;
        vec3 rayStop = eye.Origin + eye.Dir * tfar;

        // http://stackoverflow.com/questions/12494439/opacity-correction-in-raycasting-volume-rendering
        float alphaCorrection = float( maxSamplesPerRay) / float( nSamplesPerRay );

        vec3 pos = rayStart;
        vec3 step = normalize( rayStop - rayStart ) * stepSize;

        //Used later for MAD optimization in the raymarching loop
        const float multiplyer = 1 / ( dataSourceRange.g - dataSourceRange.r );
        const float addedValue = -dataSourceRange.r / ( dataSourceRange.g - dataSourceRange.r );

        // Front-to-back absorption-emission integrator
        for ( float travel = distance( rayStop, rayStart ); travel > 0.0; pos += step, travel -= stepSize )
        {
            vec3 texPos = calcTexturePositionFromAABBPos( pos );

            float density = 0;
            if( datatype == SH_UINT )
                density = texture( volumeTexUint, texPos ).r * multiplyer + addedValue;
            else if( datatype == SH_INT )
                density = texture( volumeTexInt, texPos ).r * multiplyer + addedValue;
            else if( datatype == SH_FLOAT )
                density = texture( volumeTexFloat, texPos ).r * multiplyer + addedValue;

            vec4 transferFn  = texture( transferFnTex, density );
            localResult = composite( transferFn, localResult, alphaCorrection );

            if( localResult.a > EARLY_EXIT )
                break;
        }
        brickResult += localResult;
    }

    imageStore( renderTexture, ivec2( gl_FragCoord.xy ), brickResult / nSamplesPerPixel );
}
