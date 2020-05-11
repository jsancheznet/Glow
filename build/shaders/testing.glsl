#ifdef VERTEX_SHADER

layout (location = 0) in vec3 Vertices;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
    gl_Position = Projection * View * Model * vec4(Vertices, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

layout (location = 0) out vec4 FragmentColor;
layout (location = 1) out vec4 BrightnessColor;

// IMPORTANT: The shaders _needs_ to write to Brightness color in
// order to show anything on the screen. Wasted a lot of time on this.

void main()
{
    FragmentColor = vec4(1.0, 0.0, 1.0, 0.0);
    BrightnessColor = vec4(FragmentColor.rgb, 1.0);
}

#endif
