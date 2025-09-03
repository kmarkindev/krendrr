#version 460

out vec4 FragColor;

uniform sampler2D GBufferColorTexture;
uniform sampler2D GBufferWorldPositionTexture;
uniform sampler2D GBufferWorldNormalTexture;
uniform sampler2D GBufferMetallicTexture;

uniform bool HasAmbient;
uniform vec3 AmbientColor;
uniform float AmbientIntensity;

uniform bool HasDirectional;
uniform vec3 DirectionalColor;
uniform vec3 DirectionalDir;
uniform float DirectionalIntensity;

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

    vec3 ambientLight = HasAmbient ? AmbientColor * AmbientIntensity : vec3(0.f);

    // Directional

    vec3 DirectionalLight = vec3(0.f);

    if(HasDirectional)
    {
        vec3 NormDirectionalDir = normalize(DirectionalDir);

        // diffuse

        vec3 DirectionalDiffuseLight = max(0.0, dot(-NormDirectionalDir, WorldNormal)) * DirectionalColor * DirectionalIntensity;

        // specular

        bool useBlinnPhong = true;
        int shininess = 64; // must be power of 2

        vec3 specularColor = DirectionalColor;
        float specularIntensity = MetallicIntensity;

        vec3 DirectionalSpecularLight = vec3(0.0);

        if(useBlinnPhong)
        {
            vec3 lightHalfVectorDir = normalize(-NormDirectionalDir + -CameraDir);

            float spec = pow(max(dot(WorldNormal, lightHalfVectorDir), 0.0), shininess);
            DirectionalSpecularLight = specularColor * spec * MetallicIntensity * specularIntensity;
        }
        else
        {
            // refrect expects first vector to point to reflection point
            vec3 lightReflectDir = reflect(-NormDirectionalDir, WorldNormal);

            float spec = pow(max(dot(CameraDir, lightReflectDir), 0.0), shininess);
            DirectionalSpecularLight = specularColor * spec * MetallicIntensity * specularIntensity;
        }

        DirectionalLight = DirectionalDiffuseLight + DirectionalSpecularLight;
    }

    // Combine

    FragColor = vec4(BaseColor * (DirectionalLight + ambientLight), 1.0);
}