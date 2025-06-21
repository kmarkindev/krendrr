#version 460

in vec3 position;
in vec2 uv;
in vec3 normal;

out vec4 FragColor;

uniform vec3 Color;
uniform sampler2D BaseColorTexture;
uniform sampler2D MetallicTexture;
uniform sampler2D RoughnessTexture;
uniform sampler2D NormalsTexture;

void main()
{
    FragColor = texture(BaseColorTexture, uv);
}