#ifdef VERTEX_SHADER

/*
  This set of shaders is meant to be used when rendering to an offscreen framebuffer.
  First we render to the framebuffer, and then we sample the texture that the framebuffer created to a screen-sized quad.
  This are the shaders for the screen quad that will draw the final frame to the window.
 */

layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 UV;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(Position.x, Position.y, 0.0, 1.0);
    TexCoords = UV;
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D ScreenTexture;

void main()
{
    // NOTE: Normal sampling, this is the default
    FragColor = texture(ScreenTexture, TexCoords);

    // NOTE: Negative color post processing effect
    // FragColor = vec4(vec3(1.0 - texture(ScreenTexture, TexCoords)), 1.0);

    // NOTE: Grayscale effect, achieved by averaging colors. They are
    // also weighted to take into accout that the human eye perceives
    // more green
    // FragColor = texture(ScreenTexture, TexCoords);
    // float Average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
    // FragColor = vec4(Average, Average, Average, 1.0);

    // NOTE: Kernels
    // const float Offset = 1.0 / 300.0; // Offset is a constant value that we can customize to our liking
    // vec2 Offsets[9] = vec2[]
    // (
    //     vec2(-Offset,  Offset), // top-left
    //     vec2( 0.0f,    Offset), // top-center
    //     vec2( Offset,  Offset), // top-right
    //     vec2(-Offset,  0.0f),   // center-left
    //     vec2( 0.0f,    0.0f),   // center-center
    //     vec2( Offset,  0.0f),   // center-right
    //     vec2(-Offset, -Offset), // bottom-left
    //     vec2( 0.0f,   -Offset), // bottom-center
    //     vec2( Offset, -Offset)  // bottom-right
    //  );

    // NOTE: This is a sharpen kernel
    // float Kernel[9] = float[]
    // (
    //     -1, -1, -1,
    //     -1,  9, -1,
    //     -1, -1, -1
    //  );

    // NOTE: This is a blur kernel
    // Such a blur effect creates interesting possibilities. We could
    // vary the blur amount over time for example to create the effect
    // of someone being drunk, or increase the blur whenever the main
    // character is not wearing glasses. Blurring also give us a
    // useful utility to smooth color values which we will use in
    // later tutorials.
    // float Kernel[9] = float[]
    // (
    // 1.0 / 16, 2.0 / 16, 1.0 / 16,
    // 2.0 / 16, 4.0 / 16, 2.0 / 16,
    // 1.0 / 16, 2.0 / 16, 1.0 / 16
    //  );

    // NOTE: Edge detection kernel
    // float Kernel[9] = float[]
    // (
    //     1, 1, 1,
    //     1,  -8, 1,
    //     1, 1, 1
    //  );

    // vec3 SampleTex[9];
    // for(int i = 0; i < 9; i++)
    // {
    //     SampleTex[i] = vec3(texture(ScreenTexture, TexCoords.st + Offsets[i]));
    // }
    // vec3 Col = vec3(0.0);
    // for(int i = 0; i < 9; i++)
    // {
    //     Col += SampleTex[i] * Kernel[i];
    // }
    // FragColor = vec4(Col, 1.0);
}
#endif
