#ifdef VERTEX_SHADER

layout (location = 0) in vec3 Vertices;

layout (std140) uniform CameraMatrices
{
    mat4 Projection;
    mat4 Orthographic;
    mat4 View;
};

void main()
{
    // Vertices are already in world space
    gl_Position = Projection * View * vec4(Vertices, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

layout (location = 0) out vec4 FragmentColor;
layout (location = 1) out vec4 BrightnessColor;

uniform vec3 Color;

void main()
{
    FragmentColor = vec4(Color, 1.0);
    // Debug lines should not glow
    BrightnessColor = vec4(0.0, 0.0, 0.0, 1.0);
}
#endif
