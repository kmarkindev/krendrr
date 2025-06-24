#version 460

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 position;
out vec2 uv;
out vec3 normal;

uniform mat4 MVP;
uniform mat4 ModelMatrix;

void main()
{
    uv = aUV;
    normal = mat3(transpose(inverse(ModelMatrix))) * aNormal;
    position = vec3(ModelMatrix * vec4(aPosition, 1.0));

    gl_Position = MVP * vec4(aPosition, 1.0);
}