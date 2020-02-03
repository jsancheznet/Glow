/*
  IMPORTANT This file is not to be compiled, it simple serves as a
  place to store unused functions in case i need to reimplement them.
 */

#pragma once

struct textured_quad
{
    u32 VAO;
    u32 EBO;
    u32 VertexBuffer;
    u32 TexCoordsBuffer;
    u32 TextureHandle;
    u32 Shader;
};

struct skybox
{
    u32 TextureID;
    u32 VAO;
    u32 VBO;
    u32 Shader;
};

struct render_target
{
    u32 Framebuffer; // The main opengl container for attachments
    u32 ColorBufferTexture;
    u32 BloomBufferTexture;
    u32 DepthStencilRenderbuffer;

    // Size of the offscreen buffer
    i32 Width;
    i32 Height;

    // The final screen sized Quad which is used to render to the window
    u32 ScreenQuadVAO;
    u32 ScreenQuadVBO;
    u32 ScreenQuadShader;

    f32 HDRExposure;
};

struct textured_cube
{
    u32 VAO;
    u32 VertexBuffer;
    u32 TexCoordsBuffer;
    u32 TextureHandle;
    u32 Shader;
};


// Functions //
textured_quad *CreateTexturedQuad(char *Filename, u32 Shader)
{
    Assert(Filename);

    if(glIsProgram(Shader) == GL_FALSE)
    {
        printf("Function CreateTexturedQuad: Shader parameter \"Shader\" is invalid in %s:%d\n", __FILE__, __LINE__);
        return NULL;
    }

    textured_quad *Result = (textured_quad*)Malloc(sizeof(textured_quad));
    Assert(Result);
    Result->TextureHandle = CreateOpenGLTexture(Filename);
    Result->Shader = Shader;

    f32 Vertices[] =
    {
        -0.5f, -0.5f, 0.0f, // Lower Left
        0.5f, -0.5f, 0.0f, // Lower Right
        0.5f, 0.5f, 0.0f, // Upper Right
        -0.5f, 0.5f, 0.0f, // Upper Left
    };

    u32 Indices[] =
    {
        0, 1, 2, // First Triangle
        2, 3, 0 // Second Triangle
    };

    f32 TexCoords[] =
    {
        0.0f, 0.0f, // Lower Left
        1.0f, 0.0f, // Lower Right
        1.0f, 1.0f, // Upper Right
        0.0f, 1.0f, // Upper Left
    };

    // VAO
    glGenVertexArrays(1, &Result->VAO);
    glBindVertexArray(Result->VAO);

    // Vertex Buffer
    glGenBuffers(1, &Result->VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Result->VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    // glNamedBufferData(Result->VertexBuffer, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Texture Coordinates
    glGenBuffers(1, &Result->TexCoordsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Result->TexCoordsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords), TexCoords, GL_STATIC_DRAW);
    // glNamedBufferData(Result->TexCoordsBuffer, sizeof(TexCoords), TexCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // EBO
    glGenBuffers(1, &Result->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Result->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
    // glNamedBufferData(Result->EBO, sizeof(Indices), Indices, GL_STATIC_DRAW);
    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    // Tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    SetShaderUniform(Result->Shader, "Image", 0);

    return Result;
}

void DrawTexturedQuad(textured_quad *Quad, glm::vec3 Position, glm::vec3 Size, glm::vec3 RotationAxis, f32 RotationAngle)
{
    // NOTE: Uses global variable Camera

    glUseProgram(Quad->Shader);

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(Position));
    Model = glm::scale(Model, Size);
    Model = glm::rotate(Model, glm::radians(RotationAngle), RotationAxis); // NOTE: Do we need to normalize the RotationAngle?
    SetShaderUniform(Quad->Shader, "Model", Model);
    SetShaderUniform(Quad->Shader, "View", Camera.View);
    SetShaderUniform(Quad->Shader, "Projection", Camera.Projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Quad->TextureHandle);
    glUseProgram(Quad->Shader);
    glBindVertexArray(Quad->VAO);
    glDrawElements(GL_TRIANGLES,6, GL_UNSIGNED_INT, 0);
}

textured_cube *CreateTexturedCube(u32 Shader, char *TextureFilename)
{
    Assert(TextureFilename);

    textured_cube *Result = (textured_cube*)Malloc(sizeof(textured_cube));
    Result->Shader = Shader;
    Result->TextureHandle = CreateOpenGLTexture(TextureFilename);

    f32 CubeVertices[] =
    {
        // positions          // texture Coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    glGenVertexArrays(1, &Result->VAO);
    glGenBuffers(1, &Result->VertexBuffer);
    glBindVertexArray(Result->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Result->VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), &CubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glUseProgram(Result->Shader);
    SetShaderUniform(Result->Shader, "Image", 0);

    return Result;
}

void
DrawText2DCentered(char *Text, font *Font, glm::vec2 Position, glm::vec2 Scale, glm::vec3 Color)
{
    Assert(Text);
    Assert(Font);

    glUseProgram(Font->Shader);
    glm::mat4 Identity = glm::mat4(1.0f);
    SetShaderUniform(Font->Shader, "Model", Identity);
    SetShaderUniform(Font->Shader, "View", Identity);
    SetShaderUniform(Font->Shader, "Projection", Camera.Ortho);
    SetShaderUniform(Font->Shader, "TextColor", Color);

    glActiveTexture(GL_TEXTURE0); // TODO: Read why do we need to activate textures! NOTE: read this https://community.khronos.org/t/when-to-use-glactivetexture/64913
    glBindVertexArray(Font->VAO);

    f32 TotalWidth = 0;
    f32 TotalHeight = 0;
    for(char *Ptr = Text; *Ptr != '\0'; Ptr++)
    {
        character Ch = Font->Characters[*Ptr];
        TotalWidth += (Ch.Advance >> 6) * Scale.x; // Bitshift by 6 to get value in pixels (2^6 = 64)

        if((f32)Ch.Size.y > TotalHeight)
        {
            TotalHeight = (f32)Ch.Size.y;
        }
    }
    f32 HalfWidth = TotalWidth / 2.0f;
    f32 HalfHeight = TotalHeight / 2.0f;

    // Iterate through all the characters in string
    for(char *Ptr = Text; *Ptr != '\0'; Ptr++)
    {
        character Ch = Font->Characters[*Ptr];

        // f32 XPos = Position.x + Ch.Bearing.x * Scale.x;
        // f32 YPos = Position.y - (Ch.Size.y - Ch.Bearing.y) * Scale.y;

        f32 XPos = Position.x - HalfWidth + Ch.Bearing.x * Scale.x;
        f32 YPos = Position.y - HalfHeight - (Ch.Size.y - Ch.Bearing.y) * Scale.y;

        f32 W = Ch.Size.x * Scale.x;
        f32 H = Ch.Size.y * Scale.y;

        // Update VBO for each character
        f32 QuadVertices[6][3] =
        {
            { XPos,     YPos + H, 0.0f},
            { XPos,     YPos,     0.0f},
            { XPos + W, YPos,     0.0f},
            { XPos,     YPos + H, 0.0f},
            { XPos + W, YPos,     0.0f},
            { XPos + W, YPos + H, 0.0f}
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, Ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, Font->VertexBuffer); // Update content of Vertex buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(QuadVertices), QuadVertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render  Quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        Position.x += (Ch.Advance >> 6) * Scale.x; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
DrawText2D(char *Text, font *Font, glm::vec2 Position, glm::vec2 Scale, glm::vec3 Color)
{
    Assert(Text);
    Assert(Font);

    glUseProgram(Font->Shader);
    glm::mat4 Identity = glm::mat4(1.0f);
    SetShaderUniform(Font->Shader, "Model", Identity);
    SetShaderUniform(Font->Shader, "View", Identity);
    SetShaderUniform(Font->Shader, "Projection", Camera.Ortho);
    SetShaderUniform(Font->Shader, "TextColor", Color);

    glActiveTexture(GL_TEXTURE0); // TODO: Read why do we need to activate textures! NOTE: read this https://community.khronos.org/t/when-to-use-glactivetexture/64913
    glBindVertexArray(Font->VAO);

    // Iterate through all the characters in string
    for(char *Ptr = Text; *Ptr != '\0'; Ptr++)
    {
        character Ch = Font->Characters[*Ptr];

        f32 XPos = Position.x + Ch.Bearing.x * Scale.x;
        f32 YPos = Position.y - (Ch.Size.y - Ch.Bearing.y) * Scale.y;

        f32 W = Ch.Size.x * Scale.x;
        f32 H = Ch.Size.y * Scale.y;

        // Update VBO for each character
        f32 QuadVertices[6][3] =
        {
            { XPos,     YPos + H, 0.0f},
            { XPos,     YPos,     0.0f},
            { XPos + W, YPos,     0.0f},
            { XPos,     YPos + H, 0.0f},
            { XPos + W, YPos,     0.0f},
            { XPos + W, YPos + H, 0.0f}
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, Ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, Font->VertexBuffer); // Update content of Vertex buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(QuadVertices), QuadVertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render  Quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        Position.x += (Ch.Advance >> 6) * Scale.x; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
DrawText3D(char *Text, font *Font, glm::vec3 Position, glm::vec3 Scale, glm::vec3 RotationAxis, f32 RotationAngle, glm::vec3 Color)
{
    Assert(Text);
    Assert(Font);

    // Rotate first, scale second, then translate
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::rotate(Model, glm::radians(RotationAngle), RotationAxis);
    Model = glm::scale(Model, Scale);
    Model = glm::translate(Model, Position);

    glUseProgram(Font->Shader);
    SetShaderUniform(Font->Shader, "Model", Model);
    SetShaderUniform(Font->Shader, "View", Camera.View);
    SetShaderUniform(Font->Shader, "Projection", Camera.Projection);
    SetShaderUniform(Font->Shader, "TextColor", Color);

    glActiveTexture(GL_TEXTURE0); // TODO: Read why do we need to activate textures! NOTE: read this https://community.khronos.org/t/when-to-use-glactivetexture/64913
    glBindVertexArray(Font->VAO);

    // Iterate through all the characters in string
    for(char *Ptr = Text; *Ptr != '\0'; Ptr++)
    {
        character Ch = Font->Characters[*Ptr];

        f32 XPos = Position.x + Ch.Bearing.x * Scale.x;
        f32 YPos = Position.y - (Ch.Size.y - Ch.Bearing.y) * Scale.y;

        f32 W = Ch.Size.x * Scale.x;
        f32 H = Ch.Size.y * Scale.y;

        // Update VBO for each character
        f32 QuadVertices[6][3] =
        {
            { XPos,     YPos + H, 0.0f},
            { XPos,     YPos,     0.0f},
            { XPos + W, YPos,     0.0f},
            { XPos,     YPos + H, 0.0f},
            { XPos + W, YPos,     0.0f},
            { XPos + W, YPos + H, 0.0f}
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, Ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, Font->VertexBuffer); // Update content of Vertex buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(QuadVertices), QuadVertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render  Quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        Position.x += (Ch.Advance >> 6) * Scale.x; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
DrawText3DCentered(char *Text, font *Font, glm::vec3 Position, glm::vec3 Scale, glm::vec3 RotationAxis, f32 RotationAngle, glm::vec3 Color)
{
    Assert(Text);
    Assert(Font);

    // Rotate first, scale second, then translate
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::rotate(Model, glm::radians(RotationAngle), RotationAxis);
    Model = glm::scale(Model, Scale);
    Model = glm::translate(Model, Position);

    glUseProgram(Font->Shader);
    SetShaderUniform(Font->Shader, "Model", Model);
    SetShaderUniform(Font->Shader, "View", Camera.View);
    SetShaderUniform(Font->Shader, "Projection", Camera.Projection);
    SetShaderUniform(Font->Shader, "TextColor", Color);

    glActiveTexture(GL_TEXTURE0); // TODO: Read why do we need to activate textures! NOTE: read this https://community.khronos.org/t/when-to-use-glactivetexture/64913
    glBindVertexArray(Font->VAO);

    f32 TotalWidth = 0;
    f32 TotalHeight = 0;
    for(char *Ptr = Text; *Ptr != '\0'; Ptr++)
    {
        character Ch = Font->Characters[*Ptr];
        TotalWidth += (Ch.Advance >> 6) * Scale.x; // Bitshift by 6 to get value in pixels (2^6 = 64)

        if((f32)Ch.Size.y > TotalHeight)
        {
            TotalHeight = (f32)Ch.Size.y * Scale.y;
        }
    }
    f32 HalfWidth = TotalWidth / 2.0f;
    f32 HalfHeight = TotalHeight / 2.0f;

    // Iterate through all the characters in string
    for(char *Ptr = Text; *Ptr != '\0'; Ptr++)
    {
        character Ch = Font->Characters[*Ptr];

        f32 XPos = Position.x - HalfWidth + Ch.Bearing.x * Scale.x;
        f32 YPos = Position.y - HalfHeight - (Ch.Size.y - Ch.Bearing.y) * Scale.y;

        f32 W = Ch.Size.x * Scale.x;
        f32 H = Ch.Size.y * Scale.y;

        // Update VBO for each character
        f32 QuadVertices[6][3] =
        {
            { XPos,     YPos + H, 0.0f},
            { XPos,     YPos,     0.0f},
            { XPos + W, YPos,     0.0f},
            { XPos,     YPos + H, 0.0f},
            { XPos + W, YPos,     0.0f},
            { XPos + W, YPos + H, 0.0f}
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, Ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, Font->VertexBuffer); // Update content of Vertex buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(QuadVertices), QuadVertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render  Quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        Position.x += (Ch.Advance >> 6) * Scale.x; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

render_target *CreateRenderTarget(char *ShaderFilename, i32 Width, i32 Height, b32 EnableHDR)
{
    // NOTE: If switching to fullscreen and back is slow, we can
    // create two separate textures and renderbuffers at rendertarget
    // creation, one for each window resolution. One fullscreen, the
    // other windowed. It may solve the speed issue.

    // A Framebuffer is a collection of buffers that can be used
    // as a destination for offscreen rendering. OpenGL has two
    // kinds of framebuffers: the Default Framebuffer, which is
    // provided by the OpenGL Context; and user-created
    // framebuffers called Framebuffer Objects (FBOs). The buffers
    // for default framebuffers are part of the context and
    // usually represent a window or display device. The buffers
    // for FBOs reference images from either Textures or
    // Renderbuffers; they are never directly visible.

    // Renderbuffer attachments can be Textures or Renderbuffer
    // objects. Renderbuffer objects are used when we are not
    // going to sample/read from the renderbuffer, they are stored
    // in opengl's internal format and are hard/inefficient to
    // sample/read from.

    // Frambuffer has multiple color buffers attachments + depth + stencil attachments.

    // When the window is created and opengl is initialized,
    // OpenGL automatically creates a framebuffer. We can also
    // create our own framebuffers, this will enable us to do
    // postprocessig and other cool effects.

    // NOTE: This function creates an HDR + Bloom framebuffer

    Assert(ShaderFilename);
    Assert(Width > 0);
    Assert(Height > 0);

    render_target *Result = (render_target*)Malloc(sizeof(render_target));
    Result->ScreenQuadShader = CreateShaderProgram(ShaderFilename);
    Result->Width = Width;
    Result->Height = Height;
    // Result->HDRExposure = 0.01f;
    Result->HDRExposure = 1.0f;

    // Create the main framebuffer
    glGenFramebuffers(1, &Result->Framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, Result->Framebuffer);

    { // SECTION: Color Buffer Texture
        glGenTextures(1, &Result->ColorBufferTexture);
        glBindTexture(GL_TEXTURE_2D, Result->ColorBufferTexture);
        // TODO: Create a  Multisample texture so we can support Anti Aliasing
        // TODO: Write a note of how to create multisampled framebuffers
        // TODO: Fix colors, should be SRGB
        i32 LevelOfDetail = 0;
        i32 InternalFormat = GL_RGB16F;
        GLenum FormatOfPixelData = GL_RGB;
        GLenum PixelDataType = GL_UNSIGNED_BYTE;
        glTexImage2D(GL_TEXTURE_2D, LevelOfDetail, InternalFormat, Width, Height, 0, FormatOfPixelData, PixelDataType, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Result->ColorBufferTexture, 0);
    }

    { // SECTION: Bloom Buffer Texture
        glGenTextures(1, &Result->BloomBufferTexture);
        glBindTexture(GL_TEXTURE_2D, Result->BloomBufferTexture);
        // TODO: Create a  Multisample texture so we can support Anti Aliasing
        // TODO: Write a note of how to create multisampled framebuffers
        // TODO: Fix colors, should be SRGB
        i32 LevelOfDetail = 0;
        i32 InternalFormat = GL_RGB16F;
        GLenum FormatOfPixelData = GL_RGB;
        GLenum PixelDataType = GL_UNSIGNED_BYTE;
        glTexImage2D(GL_TEXTURE_2D, LevelOfDetail, InternalFormat, Width, Height, 0, FormatOfPixelData, PixelDataType, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // Attach the texture buffer to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, Result->BloomBufferTexture, 0);
    }
    // TODO: Jorge, Does the glDrawBuffers call go here?!?!?!?!?
    u32 Attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, Attachments);

    // Create a RenderBuffer object for depth and stencil attachments.
    glGenRenderbuffers(1, &Result->DepthStencilRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, Result->DepthStencilRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Width, Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Result->DepthStencilRenderbuffer);

    // Now that all attachments are added, we check if the framebuffer is complete
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("ERROR: Framebuffer is not complete! - %s:%d\n", __FILE__, __LINE__);
        glDeleteFramebuffers(1, &Result->Framebuffer);
        glDeleteTextures(1, &Result->ColorBufferTexture);
        glDeleteTextures(1, &Result->BloomBufferTexture);
        glDeleteRenderbuffers(1, &Result->DepthStencilRenderbuffer);
        Free(Result);
        return NULL;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // VAO and Vertex Data for displaying the off-screen buffer to the main window
    // VAO for the final screen render quad
    f32 QuadVertices[] =
    { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &Result->ScreenQuadVAO);
    glGenBuffers(1, &Result->ScreenQuadVBO);
    glBindVertexArray(Result->ScreenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, Result->ScreenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), &QuadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2 * sizeof(f32)));
    SetShaderUniform(Result->ScreenQuadShader, "ScreenTexture", 0);

    return Result;
}

void SetActiveRenderTarget(render_target *RenderTarget)
{
    // Cannot assert, RenderTarget is allowed to be NULL/0
    if(RenderTarget == NULL)
    {
        // Set to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, RenderTarget->Framebuffer);
    }
}

void DisplayRenderTarget(render_target *RenderTarget)
{
    glDisable(GL_DEPTH_TEST);
    glUseProgram(RenderTarget->ScreenQuadShader);
    SetShaderUniform(RenderTarget->ScreenQuadShader, "Exposure", RenderTarget->HDRExposure);
    glBindVertexArray(RenderTarget->ScreenQuadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, RenderTarget->ColorBufferTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST); // Enable depth testing (it's disabled for rendering screen space quad)
}

void ResizeRenderTarget(render_target *RenderTarget, i32 Width, i32 Height)
{
    Assert(RenderTarget);
    Assert(Width > 0);
    Assert(Height > 0);

    // Delete old texture and depth+stencil renderbuffer
    glDeleteTextures(1, &RenderTarget->ColorBufferTexture);
    glDeleteTextures(1, &RenderTarget->BloomBufferTexture);
    glDeleteRenderbuffers(1, &RenderTarget->DepthStencilRenderbuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, RenderTarget->Framebuffer);

    { // SECTION: Color Attachment 0
        glGenTextures(1, &RenderTarget->ColorBufferTexture);
        glBindTexture(GL_TEXTURE_2D, RenderTarget->ColorBufferTexture);
        i32 LevelOfDetail = 0;
        i32 InternalFormat = GL_RGB16F;
        GLenum FormatOfPixelData = GL_RGB;
        GLenum PixelDataType = GL_UNSIGNED_BYTE;
        glTexImage2D(GL_TEXTURE_2D, LevelOfDetail, InternalFormat, Width, Height, 0, FormatOfPixelData, PixelDataType, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Attach the texture buffer to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTarget->ColorBufferTexture, 0);
    }

    { // SECTION: Bloom Texture color attachment
        glGenTextures(1, &RenderTarget->BloomBufferTexture);
        glBindTexture(GL_TEXTURE_2D, RenderTarget->BloomBufferTexture);
        i32 LevelOfDetail = 0;
        i32 InternalFormat = GL_RGB16F;
        GLenum FormatOfPixelData = GL_RGB;
        GLenum PixelDataType = GL_UNSIGNED_BYTE;
        glTexImage2D(GL_TEXTURE_2D, LevelOfDetail, InternalFormat, Width, Height, 0, FormatOfPixelData, PixelDataType, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Attach the texture buffer to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, RenderTarget->ColorBufferTexture, 0);
    }

    // RenderBuffer
    glGenRenderbuffers(1, &RenderTarget->DepthStencilRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, RenderTarget->DepthStencilRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Width, Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RenderTarget->DepthStencilRenderbuffer);

    // Now that all attachments are added, we check if the framebuffer is complete
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("ERROR: Framebuffer is not complete!\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}




skybox *CreateSkybox(u32 SkyboxShader, char *Right, char *Left, char *Top, char *Bottom, char *Front, char *Back)
{
    Assert(Right);
    Assert(Left);
    Assert(Top);
    Assert(Bottom);
    Assert(Front);
    Assert(Back);

    skybox *Result = NULL;
    Result = (skybox*)Malloc(sizeof(skybox));
    if(Result)
    {
        if(glIsProgram(SkyboxShader))
        {
            Result->Shader = SkyboxShader;
        }
        else
        {
            // Shader Parameter is invalid
            Free(Result);
            return NULL;
        }

        // VAO, VBO, Upload Vertex data
        f32 SkyboxVertices[] =
        {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
        };
        glGenVertexArrays(1, &Result->VAO);
        glGenBuffers(1, &Result->VBO);
        glBindVertexArray(Result->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, Result->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxVertices), &SkyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        // Load images, Create cubemap texture
        glGenTextures(1, &Result->TextureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Result->TextureID);

        char *Filenames[] =
        {
            Right,
            Left,
            Top,
            Bottom,
            Front,
            Back,
        };

        i32 CubeFaceCount = 6;
        for(i32 i = 0; i < CubeFaceCount; i++)
        {
            i32 RequestedChannelCount = 0;
            i32 Width, Height, ChannelCount;
            GLenum InputFormat, OutputFormat;
            stbi_set_flip_vertically_on_load(0);
            u8 *Data = stbi_load(Filenames[i], &Width, &Height, &ChannelCount, RequestedChannelCount);
            if(Data)
            {
                if(ChannelCount == 3)
                {
                    InputFormat = GL_SRGB;
                    OutputFormat = GL_RGB;
                }
                else if(ChannelCount == 4)
                {
                    InputFormat = GL_SRGB_ALPHA;
                    OutputFormat = GL_RGBA;
                }
                else
                {
                    fprintf(stderr, "%s Texture format is not GL_RGB or GL_RGBA\n", Filenames[i]);
                    free(Result);
                    stbi_image_free(Data);
                    return NULL;
                }
                // REMINDER: Textures used for data should not be upload with SRGB Format
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, InputFormat, Width, Height, 0, OutputFormat, GL_UNSIGNED_BYTE, Data);
                stbi_image_free(Data);
            }
            else
            {
                printf("CreateSkybox: failed to load %s in %s:%d\n", Filenames[i], __FILE__, __LINE__);
                Free(Result);
                stbi_image_free(Data);
                return NULL;
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
    else
    {
        // Failed to allocate memory
        return NULL;
    }

    // NOTE: Shader Configuration
    SetShaderUniform(SkyboxShader, "Skybox", 0);

    return Result;

}

void DrawSkybox(skybox *Skybox)
{
    // TODO(Jorge): Optimize the skybox rendering by doing the section
    // "An Optimization":
    // https://learnopengl.com/Advanced-OpenGL/Cubemaps
    glDepthMask(GL_FALSE);
    glUseProgram(Skybox->Shader);
    glm::mat4 View = glm::mat4(glm::mat3(Camera->View)); // Take the translation part of the mat4 by casting converting it to mat3
    SetShaderUniform(Skybox->Shader, "View", View);
    SetShaderUniform(Skybox->Shader, "Projection", Camera.Projection);
    // skybox cube
    glBindVertexArray(Skybox->VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox->TextureID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
}

void DrawTexturedCube(textured_cube *Cube, glm::vec3 Position, glm::vec3 Scale, glm::vec3 RotationAxis, f32 RotationAngle)
{
    glUseProgram(Cube->Shader);
    glBindVertexArray(Cube->VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Cube->TextureHandle);
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Position);
    Model = glm::scale(Model, Scale);
    Model = glm::rotate(Model, glm::radians(RotationAngle), RotationAxis);
    SetShaderUniform(Cube->Shader, "Model", &Model);
    SetShaderUniform(Cube->Shader, "View", &Camera.View);
    SetShaderUniform(Cube->Shader, "Projection", &Camera.Projection);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}
