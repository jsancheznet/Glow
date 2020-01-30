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

in vec2 TexCoords;

out vec4 FragmentColor;

uniform sampler2D HDRBuffer;
uniform float Exposure;
uniform bool HDR;

void main()
{
    const float Gamma = 2.2;
    vec3 HDRColor = texture(HDRBuffer, TexCoords).rgb;

    if(HDR)
    {
        // Exposure
        vec3 Result = vec3(1.0) - exp(-HDRColor * Exposure);
        // Gamma Correction
        Result = pow(Result, vec3(1.0 / Gamma));
        FragmentColor = vec4(Result, 1.0);
    }
    else
    {
        vec3 Result = pow(HDRColor, vec3(1.0 / Gamma));
        FragmentColor = vec4(Result, 1.0);
    }
}
#endif
