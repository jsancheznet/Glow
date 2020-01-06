#ifdef VERTEX_SHADER

layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 UV;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(Position.x, Position.y, 0.0, 1.0);
    TexCoords = UV;
}

#endif
#ifdef FRAGMENT_SHADER
in vec2 TexCoords;

uniform sampler2D InputBuffer;
uniform float Exposure;

out vec4 FragColor;

void main()
{
    const float Gamma = 2.2;
    vec3 HDRColor = texture(InputBuffer, TexCoords).rgb;

    // Reinhard tone mapping
    vec3 Mapped = vec3(1.0) - exp(-HDRColor * Exposure);

    // Gamma correction
    Mapped = pow(Mapped, vec3(1.0 / Gamma));

    FragColor = vec4(Mapped, 1.0);
}
#endif
