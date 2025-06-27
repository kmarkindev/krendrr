#version 460

in vec3 position;
in vec2 uv;
in vec3 normal;
in mat3 TBN;

out vec4 FragColor;

uniform vec3 Color;
uniform sampler2D BaseColorTexture;
uniform sampler2D MetallicTexture;
uniform sampler2D RoughnessTexture;
uniform sampler2D NormalsTexture;

uniform vec3 CameraPos;

void main()
{
    vec3 lightPos = vec3(0, 0, 100);

    // Normal Mapping

    vec3 sampledNormal = texture(NormalsTexture, uv).rgb;
    sampledNormal = normalize(sampledNormal * 2.0 - 1.0);
    vec3 normalNorm = normalize(TBN * sampledNormal);

    // Ambient

    float ambientIntensity = 0.1;
    vec3 ambientColor = vec3(1.0, 1.0, 1.0);

    vec3 ambientLightcolor = ambientColor * ambientIntensity;

    // Diffuse

    vec3 diffuseColor = vec3(1.0, 1.0, 1.0);

    vec3 lightDir = normalize(lightPos - position);

    float diffuseLightIntensity = max(0.0, dot(normalNorm, lightDir));
    vec3 diffuseResultColor = diffuseColor * diffuseLightIntensity;

    // Specular

    bool useBlinnPhong = true;
    int shininess = 32; // must be power of 2

    float specularIntensity = texture(MetallicTexture, uv).r;
    vec3 specularColor = vec3(1.0, 1.0, 1.0);

    vec3 cameraDir = normalize(CameraPos - position);

    vec3 specularLightColor = vec3(0.0, 0.0, 0.0);

    if(useBlinnPhong)
    {
        vec3 lightHalfVectorDir = normalize(lightDir + cameraDir);

        float spec = pow(max(dot(normalNorm, lightHalfVectorDir), 0.0), shininess);
        specularLightColor = specularColor * spec * specularIntensity;
    }
    else
    {
        // refrect expects first vector to point to reflection point
        vec3 lightReflectDir = reflect(-lightDir, normalNorm);

        float spec = pow(max(dot(cameraDir, lightReflectDir), 0.0), shininess);
        specularLightColor = specularColor * spec * specularIntensity;
    }

    // Combine

    vec3 objectColor = texture(BaseColorTexture, uv).rgb;

    FragColor = vec4((ambientLightcolor + diffuseResultColor + specularLightColor) * objectColor, 1.0);
}