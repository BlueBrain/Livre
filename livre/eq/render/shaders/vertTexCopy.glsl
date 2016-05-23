/*
 * Copyright (c) 2016  Ahmet Bilgili
 */

#version 330
#extension GL_EXT_gpu_shader4 : enable

layout( location = 0 ) in vec3 vertex_position;

void main(void)
{
    gl_Position = vec4( vertex_position, 1.0 );
}
