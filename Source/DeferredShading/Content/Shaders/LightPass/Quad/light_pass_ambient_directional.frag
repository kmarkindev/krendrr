#version 460

out vec4 FragColor;

uniform sampler2D GBufferColorTexture;
uniform sampler2D GBufferWorldPositionTexture;
uniform sampler2D GBufferWorldNormalTexture;
uniform sampler2D GBufferMetallicTexture;
uniform vec2 ScreenSize;

void main()
{
    vec2 GBufferUv = gl_FragCoord.xy / ScreenSize;
    vec3 BaseColor = texture(GBufferColorTexture, GBufferUv).rgb;
    vec3 WorldPosition = texture(GBufferWorldPositionTexture, GBufferUv).rgb;
    vec3 WorldNormal = texture(GBufferWorldNormalTexture, GBufferUv).rgb;
    float MetallicIntensity = texture(GBufferMetallicTexture, GBufferUv).r;

    // Ambient

    float ambientIntensity = 0.05;
    vec3 ambientColor = vec3(1.0, 1.0, 1.0);

    vec3 ambientLight = ambientColor * ambientIntensity;

    // Directional

    vec3 DirectionalDir = normalize(vec3(-0.2f, -1.0f, -0.3f));
    float DirectionalIntensity = 1.0;
    vec3 DirectionalColor = vec3(1.0, 1.0, 1.0);

    vec3 DirectionalLight = max(0.0, dot(-DirectionalDir, WorldNormal)) * DirectionalColor * DirectionalIntensity;

    // Combine

    FragColor = vec4(BaseColor * (DirectionalLight + ambientLight), 1.0);
}