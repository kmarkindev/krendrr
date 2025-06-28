#version 460

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragWorldPosition;
layout (location = 2) out vec4 FragWorldNormal;
layout (location = 3) out vec4 FragMetallic;

in vec3 oWorldPosition;
in vec2 oUv;
in mat3 oTBNMatrix;

uniform sampler2D BaseColorTexture;
uniform sampler2D MetallicTexture;
uniform sampler2D NormalTexture;

void main()
{
    FragColor = vec4(texture(BaseColorTexture, oUv).rgb, 1.0);
    FragWorldPosition = vec4(oWorldPosition, 1.0);
    FragMetallic.r = texture(MetallicTexture, oUv).r;
    FragMetallic = vec4(FragMetallic.r, FragMetallic.r, FragMetallic.r, 1.0);

    vec3 SampledNormal = texture(NormalTexture, oUv).rgb;
    SampledNormal = SampledNormal * 2 - 1.0;

    FragWorldNormal = vec4(oTBNMatrix * SampledNormal, 1.0);
}