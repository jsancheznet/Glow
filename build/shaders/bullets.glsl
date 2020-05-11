#ifdef VERTEX_SHADER

layout (location = 0) in vec3 Vertices;
layout (location = 1) in vec2 TexCoords;
layout (location = 3) in mat4 InstanceMatrix;

uniform mat4 View;
uniform mat4 Projection;

out vec2 TextureCoordinates;

void main()
{
    TextureCoordinates = TexCoords;
    gl_Position = Projection * View * InstanceMatrix * vec4(Vertices, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

layout (location = 0) out vec4 FragmentColor;
layout (location = 1) out vec4 BrightnessColor;

in vec2 TextureCoordinates;
uniform sampler2D Image;
uniform float BrightnessThreshold;

// IMPORTANT: The shaders _needs_ to write to Brightness color in
// order to show anything on the screen. Wasted a lot of time on this.

void main()
{
    FragmentColor = texture(Image, TextureCoordinates);

    // check whether fragment output is higher than threshold, if so output as brightness color
    float Brightness = dot(FragmentColor.rgb, vec3(0.2126, 0.7152, 0.0722));

    if(Brightness > BrightnessThreshold)
    {
        BrightnessColor = vec4(FragmentColor.rgb, 1.0);
    }
    else
    {
        BrightnessColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
#endif
