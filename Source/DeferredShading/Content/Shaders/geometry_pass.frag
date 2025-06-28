#version 460

layout (location = 0) out vec4 FragColor;

in vec3 oWorldPosition;
in vec2 oUv;
in mat3 oTBNMatrix;

uniform sampler2D BaseColorTexture;

void main()
{
    FragColor = vec4(texture(BaseColorTexture, oUv).rgb, 1.0);
}