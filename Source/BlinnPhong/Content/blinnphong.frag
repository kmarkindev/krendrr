#version 460

in vec3 position;
in vec2 uv;
in vec3 normal;

out vec4 FragColor;

uniform vec3 Color;

void main()
{
    FragColor = vec4(Color, 1.0);
}