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
     vec2 Tex_Offset = 1.0 / textureSize(Image, 0); // gets size of single texel
     vec3 Result = texture(Image, TexCoords).rgb * Weight[0];
     if(Horizontal)
     {
         for(int i = 1; i < 5; ++i)
         {
            Result += texture(Image, TexCoords + vec2(Tex_Offset.x * i, 0.0)).rgb * Weight[i];
            Result += texture(Image, TexCoords - vec2(Tex_Offset.x * i, 0.0)).rgb * Weight[i];
         }
     }
     else
     {
         for(int i = 1; i < 5; ++i)
         {
             Result += texture(Image, TexCoords + vec2(0.0, Tex_Offset.y * i)).rgb * Weight[i];
             Result += texture(Image, TexCoords - vec2(0.0, Tex_Offset.y * i)).rgb * Weight[i];
         }
     }
     FragmentColor = vec4(Result, 1.0);
}
#endif
