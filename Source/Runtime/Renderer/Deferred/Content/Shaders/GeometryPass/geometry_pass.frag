#version 460

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 FragWorldPosition;
layout (location = 2) out vec3 FragWorldNormal;
layout (location = 3) out float FragMetallic;
layout (location = 4) out float FragRoughness;
layout (location = 5) out vec4 FragEmissive;

in vec3 oWorldPosition;
in vec2 oUv;
in mat3 oTBNMatrix;
in vec3 oNormal;

uniform sampler2D BaseColorTexture;
uniform sampler2D MetallicTexture;
uniform sampler2D NormalTexture;
uniform sampler2D RoughnessTexture;
uniform sampler2D EmissiveTexture;

void main()
{
    FragColor = vec4(texture(BaseColorTexture, oUv).rgb, 1.0);
    FragWorldPosition = oWorldPosition;
    FragMetallic = texture(MetallicTexture, oUv).r;
    FragRoughness = texture(RoughnessTexture, oUv).r;

    // We NEED to save alpha for emission
    FragEmissive = texture(EmissiveTexture, oUv);
    FragEmissive.a = length(FragEmissive.rgb) == 0.f ? 0.f : 1.f;

    vec3 SampledNormal = texture(NormalTexture, oUv).rgb;
    SampledNormal = normalize(SampledNormal * 2.0 - 1.0);
    FragWorldNormal = oTBNMatrix * SampledNormal;

    // Convert from DirectX normal map to OpenGl normal map (left handed system to right handed)
    // We convert Z, since Y and Z were swapped during normals and tangents loading
    FragWorldNormal.z = -FragWorldNormal.z;
}