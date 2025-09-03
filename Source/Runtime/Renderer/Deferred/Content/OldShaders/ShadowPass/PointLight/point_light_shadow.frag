#version 460

layout (location = 0) out float FragColor;

in vec3 Position;

uniform float FarDistance;
uniform vec3 LightPosition;

void main()
{
    FragColor = length(Position - LightPosition) / FarDistance;
}