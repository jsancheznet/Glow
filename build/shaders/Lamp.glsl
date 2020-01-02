#ifdef VERTEX_SHADER

layout (location = 0) in vec3 Position;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
	gl_Position = Projection * View * Model * vec4(Position, 1.0);
}
#endif

#ifdef FRAGMENT_SHADER

out vec4 FragColor;

uniform vec3 LightColor;

void main()
{
    FragColor = vec4(LightColor, 1.0); // set all 4 vector values to 1.0 (White)
}
#endif
