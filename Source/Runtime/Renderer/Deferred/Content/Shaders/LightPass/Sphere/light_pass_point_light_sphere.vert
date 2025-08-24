#version 460

layout (location = 0) in vec3 aPosition;

uniform mat4 MVPMatrix;

void main()
{
    gl_Position = MVPMatrix * vec4(aPosition, 1.0);
}