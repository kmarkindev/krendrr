#version 460

out vec4 FragColor;

uniform sampler2D GBufferColorTexture;
uniform sampler2D GBufferWorldPositionTexture;
uniform sampler2D GBufferWorldNormalTexture;
uniform sampler2D GBufferMetallicTexture;
uniform vec2 ScreenSize;
uniform vec3 CameraPos;

void main()
{
    vec2 GBufferUv = gl_FragCoord.xy / ScreenSize;
    vec3 BaseColor = texture(GBufferColorTexture, GBufferUv).rgb;
    vec3 WorldPosition = texture(GBufferWorldPositionTexture, GBufferUv).rgb;
    vec3 WorldNormal = texture(GBufferWorldNormalTexture, GBufferUv).rgb;
    float MetallicIntensity = texture(GBufferMetallicTexture, GBufferUv).r;
    vec3 CameraDir = normalize(CameraPos - WorldPosition);

    // Ambient

    float ambientIntensity = 0.005;
    vec3 ambientColor = vec3(1.0, 1.0, 1.0);

    vec3 ambientLight = ambientColor * ambientIntensity;

    // Directional

    vec3 DirectionalDir = normalize(vec3(0.0f, -1.0f, -1.0f));

    // diffuse

    float DirectionalIntensity = 0.0;// 0.15;
    vec3 DirectionalColor = vec3(1.0, 1.0, 1.0);

    vec3 DirectionalDiffuseLight = max(0.0, dot(-DirectionalDir, WorldNormal)) * DirectionalColor * DirectionalIntensity;

    // specular

    bool useBlinnPhong = true;
    int shininess = 64; // must be power of 2

    vec3 specularColor = vec3(1.0, 1.0, 1.0);
    float specularIntensity = 0.0;

    vec3 DirectionalSpecularLight = vec3(0.0);

    if(useBlinnPhong)
    {
        vec3 lightHalfVectorDir = normalize(-DirectionalDir + -CameraDir);

        float spec = pow(max(dot(WorldNormal, lightHalfVectorDir), 0.0), shininess);
        DirectionalSpecularLight = specularColor * spec * MetallicIntensity * specularIntensity;
    }
    else
    {
        // refrect expects first vector to point to reflection point
        vec3 lightReflectDir = reflect(-DirectionalDir, WorldNormal);

        float spec = pow(max(dot(CameraDir, lightReflectDir), 0.0), shininess);
        DirectionalSpecularLight = specularColor * spec * MetallicIntensity * specularIntensity;
    }

    vec3 DirectionalLight = DirectionalDiffuseLight + DirectionalSpecularLight;

    // Combine

    FragColor = vec4(BaseColor * (DirectionalLight + ambientLight), 1.0);
}