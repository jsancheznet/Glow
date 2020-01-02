#ifdef VERTEX_SHADER
layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 UV;

out vec2 TexCoords;

void main()
{
    TexCoords = UV;
    gl_Position = vec4(Position, 1.0);
}
#endif

#ifdef FRAGMENT_SHADER
out vec4 FragmentColor;

in vec2 TexCoords;

uniform sampler2D Image;

uniform bool Horizontal;
uniform float Weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{
     vec2 tex_offset = 1.0 / textureSize(Image, 0); // gets size of single texel
     vec3 result = texture(Image, TexCoords).rgb * Weight[0];
     if(Horizontal)
     {
         for(int i = 1; i < 5; ++i)
         {
            result += texture(Image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * Weight[i];
            result += texture(Image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * Weight[i];
         }
     }
     else
     {
         for(int i = 1; i < 5; ++i)
         {
             result += texture(Image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * Weight[i];
             result += texture(Image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * Weight[i];
         }
     }
     FragmentColor = vec4(result, 1.0);
}
#endif
