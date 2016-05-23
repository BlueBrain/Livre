/*
 * Copyright (c) 2016  Ahmet Bilgili
 */

#version 420 core
#extension GL_ARB_texture_rectangle : enable

layout( location = 0 ) out vec4 FragColor;
layout( rgba32f ) coherent uniform image2DRect renderTexture;

void main()
{
    FragColor = imageLoad( renderTexture, ivec2( gl_FragCoord.xy ));
    imageStore( renderTexture, ivec2( gl_FragCoord.xy ), vec4( 0.0 ));
}
