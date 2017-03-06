#version 330

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 vertexNormal2;
layout(location = 3) in vec4 vertexColor;
layout(location = 4) in uint vertexType;

flat out vec3 normal;
flat out vec3 normal2;
flat out vec3 eyePos;
out vec4 color;
flat out uint type;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelView;
uniform mat4 normalMatrix;

void main()
{
    gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1.0);
    normal = vec3(normalMatrix * vec4(normalize(vertexNormal), 0.0));
    normal2 = vec3(normalMatrix * vec4(normalize(vertexNormal2), 0.0));
    eyePos = vec3(modelView * vec4(vertexPosition, 1.0));
    color = vertexColor;
    type = vertexType;
}
