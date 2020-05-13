#ifdef VERTEX_SHADER

layout(location = 0) in vec3 Vertices;
layout(location = 1) in vec2 TexCoords;

//  Variables in a uniform block can be directly accessed without the
//  block name as a prefix.
layout (std140) uniform CameraMatrices
{
    mat4 CameraProjection;
    mat4 CameraOrthographic;
    mat4 CameraView;
};

uniform mat4 Model;
uniform mat4 View; // TODO: Delete me
uniform mat4 Projection; // TODO: Delete me

out vec2 UV;

void main()
{
    // gl_Position = Projection * View * Model * vec4(Vertices, 1.0);
    gl_Position = Projection * View * Model * vec4(Vertices, 1.0);
    UV = TexCoords;
}

#endif

#ifdef FRAGMENT_SHADER

layout (location = 0) out vec4 FragmentColor;
layout (location = 1) out vec4 BrightnessColor;

in vec2 UV;
uniform sampler2D Text;
uniform vec3 TextColor;
uniform float BrightnessThreshold;

// IMPORTANT: The shaders _needs_ to write to Brightness color in
// order to show anything on the screen. Wasted a lot of time on this.

void main()
{
    vec4 Sampled = vec4(1.0, 1.0, 1.0, texture(Text, UV).r);
    FragmentColor = vec4(TextColor, 1.0) * Sampled;

    float Brightness = dot(FragmentColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(Brightness > BrightnessThreshold)
    {
        BrightnessColor = FragmentColor;
    }
    else
    {
        BrightnessColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}

#endif
