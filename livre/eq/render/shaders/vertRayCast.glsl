/*
 * Copyright (c) 2007       Maxim Makhinya
 */

#version 110

// updated per frame
void main(void)
{
    gl_Position = ftransform();
    gl_FrontColor = gl_Color;
}
