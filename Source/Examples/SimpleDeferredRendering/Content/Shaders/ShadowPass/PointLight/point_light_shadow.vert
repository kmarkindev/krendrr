#version 460

layout (location = 0) in vec3 aPosition;

uniform mat4 MVPMatrix;
uniform mat4 ModelMatrix;

out vec3 Position;

void main()
{
    vec4 HomogenusWorldPos = ModelMatrix * vec4(aPosition, 1.0);
    Position = HomogenusWorldPos.xyz / HomogenusWorldPos.w;

    gl_Position = MVPMatrix * vec4(aPosition, 1.0);
}