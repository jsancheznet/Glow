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

in vec2 TextureCoordinates;
uniform sampler2D Image;
out vec4 FragColor;

void main()
{
    FragColor = texture(Image, TextureCoordinates);
    // Apply Gamma correction
    // REMINDER: We are applying Gamma Correction by glEnable(SRGB_FRAMEUBFFER)
    // float Gamma = 2.2;
    // FragColor.rgb = pow(FragColor.rgb, vec3(1.0/Gamma));
}

#endif
