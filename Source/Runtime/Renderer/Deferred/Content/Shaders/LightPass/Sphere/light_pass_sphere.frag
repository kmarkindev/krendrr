#version 460

uniform sampler2D GBufferColorTexture;
uniform sampler2D GBufferWorldPositionTexture;
uniform sampler2D GBufferWorldNormalTexture;
uniform sampler2D GBufferMetallicTexture;
uniform samplerCube PointLightShadowCubeMap;
uniform vec2 ScreenSize;
uniform vec3 CameraPos;

uniform float PointLightFarPlane;

uniform vec3 PointLightPosition;
uniform vec3 PointLightDiffuseColor;
uniform vec3 PointLightSpecularColor;
uniform float PointLightAttenuationLinear;
uniform float PointLightAttenuationQuad;
uniform float PointLightAttenuationConstant;

out vec4 FragColor;

void main()
{
    vec2 GBufferUv = gl_FragCoord.xy / ScreenSize;
    vec3 WorldPosition = texture(GBufferWorldPositionTexture, GBufferUv).rgb;
    vec3 WorldNormal = texture(GBufferWorldNormalTexture, GBufferUv).rgb;

    vec3 LightDir = normalize(WorldPosition - PointLightPosition);

    float ShadowMapDepthDistance = texture(PointLightShadowCubeMap, LightDir).r * PointLightFarPlane;
    float PointLightDistance = length(PointLightPosition - WorldPosition);
    float Attenuation = 1.0 / (PointLightAttenuationConstant + PointLightAttenuationLinear * PointLightDistance + PointLightAttenuationQuad * (PointLightDistance * PointLightDistance));

    // diffuse

    float DiffuseScale = max(0.0, dot(-LightDir, WorldNormal));

    vec3 ResultDiffuseColor = max(PointLightDiffuseColor * DiffuseScale * Attenuation, 0.0);

    // specular

    vec3 ResultSpecularColor = vec3(0, 0, 0); //vec4(PointLightSpecularColor * Attenuation, 1);

    // combine

    vec3 BaseColor = texture(GBufferColorTexture, GBufferUv).rgb;

    FragColor = vec4(BaseColor * (ResultDiffuseColor + ResultSpecularColor), 1);

    // apply shadow
    float ShadowValue = ShadowMapDepthDistance <= PointLightDistance ? 1.f : 0.f;

    // TODO: add PFC for shadow

    FragColor *= 1.0 - ShadowValue;
}