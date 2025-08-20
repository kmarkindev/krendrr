#version 460

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aUv;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

out vec3 oWorldPosition;
out vec2 oUv;
out mat3 oTBNMatrix;
out vec3 oNormal;

uniform mat3 NormalMatrix;
uniform mat4 ModelMatrix;
uniform mat4 MVPMatrix;

void main()
{
    oUv = aUv;

    vec4 HomogenusWorldPos = ModelMatrix * vec4(aPosition, 1.0);
    oWorldPosition = HomogenusWorldPos.xyz / HomogenusWorldPos.w;

    vec3 Normal = normalize(NormalMatrix * aNormal);
    vec3 Tangent = normalize(NormalMatrix * aTangent);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal); // Gram-Schmidt
    vec3 Bitangent = cross(Normal, Tangent);
    oTBNMatrix = mat3(Tangent, Bitangent, Normal);

    oNormal = Normal;
    gl_Position = MVPMatrix * vec4(aPosition, 1.0);
}