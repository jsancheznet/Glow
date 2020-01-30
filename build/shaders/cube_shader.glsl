#ifdef VERTEX_SHADER
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 UV;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

out vec2 TextureCoordinates;

void main()
{
    // vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    // vs_out.TexCoords = aTexCoords;
    // mat3 normalMatrix = transpose(inverse(mat3(model)));
    // vs_out.Normal = normalize(normalMatrix * aNormal);

    TextureCoordinates = UV;
    gl_Position = Projection * View * Model * vec4(Position, 1.0);

}
#endif
#ifdef FRAGMENT_SHADER

layout (location = 0) out vec4 FragmentColor;
layout (location = 1) out vec4 BrightnessColor; // IMPORTANT: This shader also outputs to the brightness texture

uniform vec3 Color;

void main()
{
    FragmentColor = vec4(Color, 1.0);

    // check whether fragment output is higher than threshold, if so output as brightness color
    float Brightness = dot(FragmentColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(Brightness > 1.0)
    {
        BrightnessColor = vec4(FragmentColor.rgb, 1.0);
    }
    else
    {
        BrightnessColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}

#endif
