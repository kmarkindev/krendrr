#version 460

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aUv;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

out vec3 oWorldPosition;
out vec2 oUv;
out mat3 oTBNMatrix;

uniform mat4 NormalMatrix;
uniform mat4 ModelMatrix;
uniform mat4 MVPMatrix;

void main()
{
    oUv = aUv;
    oWorldPosition = (ModelMatrix * vec4(aPosition, 1.0)).xyz;

    vec3 Normal = (NormalMatrix * vec4(aNormal, 0.0)).xyz;
    vec3 Tangent = (NormalMatrix * vec4(aTangent, 0.0)).xyz;
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal); // Gram-Schmidt
    vec3 Bitangent = cross(Normal, Tangent);
    oTBNMatrix = mat3(Tangent, Bitangent, Normal);

    gl_Position = MVPMatrix * vec4(aPosition, 1.0);
}