#ifdef VERTEX_SHADER
layout (location = 0) in vec3 Position;

out vec3 TexCoords;

uniform mat4 Projection;
uniform mat4 View;

void main()
{
    TexCoords = Position;
    gl_Position = Projection * View * vec4(Position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube Skybox;

void main()
{
    FragColor = texture(Skybox, TexCoords);
}

#endif
