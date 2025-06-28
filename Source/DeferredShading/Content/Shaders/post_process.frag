#version 460

out vec4 FragColor;

uniform sampler2D FinalRenderTexture;
uniform vec2 ScreenSize;

void main()
{
    vec2 Uv = gl_FragCoord.xy / ScreenSize;
    FragColor = vec4(texture(FinalRenderTexture, Uv).rgb, 1.0);

    float Gamma = 2.2;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/Gamma));
}