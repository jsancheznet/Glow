#ifdef VERTEX_SHADER

layout (location = 0) in vec3 Vertices;
layout (location = 1) in vec2 TexCoords;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec2 TextureCoordinates;

void main()
{
    TextureCoordinates = TexCoords;
    gl_Position = Projection * View * Model * vec4(Vertices, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

layout (location = 0) out vec4 FragmentColor;
layout (location = 1) out vec4 BrightnessColor; // IMPORTANT: This shader also outputs to the brightness texture

in vec2 TextureCoordinates;
uniform sampler2D Image;

void main()
{
    FragmentColor = texture(Image, TextureCoordinates);

    // check whether fragment output is higher than threshold, if so output as brightness color
    float Brightness = dot(FragmentColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(Brightness > 0.1)
    {
        BrightnessColor = vec4(FragmentColor.rgb, 1.0);
    }
    else
    {
        BrightnessColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
#endif
