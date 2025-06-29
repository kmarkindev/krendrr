#version 460

out vec4 FragColor;

uniform sampler2D FinalRenderTexture;
uniform vec2 ScreenSize;

void main()
{
    vec2 Uv = gl_FragCoord.xy / ScreenSize;
    vec3 HdrColor = texture(FinalRenderTexture, Uv).rgb;

    // Reinhard tone mapping
    HdrColor = HdrColor / (HdrColor + vec3(1.0));

    // Gamma
    float Gamma = 2.2;
    FragColor = vec4(pow(HdrColor, vec3(1.0/Gamma)), 1.0);
}