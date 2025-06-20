#version 460

in vec2 texCoords;

out vec4 FragColor;

uniform vec3 Color;
uniform sampler2D texture1;

void main()
{
    FragColor = mix(texture(texture1, texCoords), vec4(Color, 1.0), 0.2);
}