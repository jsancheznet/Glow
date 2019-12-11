#ifdef VERTEX_SHADER

layout(location = 0) in vec3 Vertices;
layout(location = 1) in vec2 TexCoords;

out vec2 UV;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
    gl_Position = Projection * View * Model * vec4(Vertices, 1.0);
    UV = TexCoords;
}

#endif

#ifdef FRAGMENT_SHADER

in vec2 UV;
out vec4 Color;

uniform sampler2D Text;
uniform vec3 TextColor;

void main()
{
    vec4 Sampled = vec4(1.0, 1.0, 1.0, texture(Text, UV).r);
    Color = vec4(TextColor, 1.0) * Sampled;
}

#endif
