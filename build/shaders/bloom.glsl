#ifdef VERTEX_SHADER

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 UV;

out vec2 TexCoords;

void main()
{
    TexCoords = UV;
    gl_Position = vec4(Position, 1.0);
}
#endif
#ifdef FRAGMENT_SHADER

out vec4 FragmentColor;
in vec2 TexCoords;

uniform sampler2D Scene;
uniform sampler2D BloomBlur;
uniform bool Bloom;
uniform float Exposure;

void main()
{
    const float Gamma = 2.2;
    vec3 HDRColor = texture(Scene, TexCoords).rgb;
    vec3 BloomColor = texture(BloomBlur, TexCoords).rgb;

    if(Bloom)
    {
        HDRColor += BloomColor; // additive blending
    }

    // tone mapping
    vec3 Result = vec3(1.0) - exp(-HDRColor * Exposure);
    // also gamma correct while we're at it
    Result = pow(Result, vec3(1.0 / Gamma));
    FragmentColor = vec4(Result, 1.0);
}
#endif
