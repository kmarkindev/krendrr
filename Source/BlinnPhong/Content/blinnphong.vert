#version 460

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

out vec3 position;
out vec2 uv;
out vec3 normal;
out mat3 TBN;

uniform mat4 MVP;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;

void main()
{
    uv = aUV;
    normal = normalize(NormalMatrix * aNormal);
    position = vec3(ModelMatrix * vec4(aPosition, 1.0));

    // Find TBL matrix
    vec3 tangent = normalize(NormalMatrix * aTangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal); // Gram-Schmidt
    vec3 bitangent = cross(normal, tangent);
    TBN = mat3(tangent, bitangent, normal);

    gl_Position = MVP * vec4(aPosition, 1.0);
}