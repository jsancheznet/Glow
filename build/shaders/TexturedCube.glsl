#ifdef VERTEX_SHADER
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = Projection * View * Model * vec4(aPos, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D Texture1;

void main()
{
    FragColor = texture(Texture1, TexCoords);
}

#endif
