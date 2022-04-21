/*

  There are several functions which start with a capital letter plus an underscore. I did that to separate subsystems, here's a list of the subsystems

  P_ - P stands for platform layer (it's not technically a platform layer but naming things is hard), functions are located in platform.cpp
  R_ - R stands for renderer, everything renderer is in renderer.cpp
  I_ - I stands for input, everything mouse and keyboard is here, input.cpp
  S_ - S stands for Sound, everything sound related is in sound.cpp
  C_ - C stands for collision, everything related to collision is inside collision.cpp
  E_ - E stands for entity, everything related is in entity.cpp
  random.cpp - contains the random number generator


  --- Game loop overall layout

  There are 4 main states in which the game might be in.
    - Initial (The first screen, the one that pop ups when the game is executed)
    - Game (This is the game)
    - Pause (Pause screen)
    - GameOver (When you die/lose)

    the game loop consists of 3 switch statements, one for input, one
    for update and one for render. so the main loop looks something
    like the following.

    Game Loop
        switch(Game state)
        {
          Do Input for player and AI
        }

        switch(Game state)
        {
          Do update for all entities
        }

        switch(Game state)
        {
          Draw things
        }
    End Game Loop

 */

#include <stdio.h>

#include "external/glad.c"
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>

#include "shared.h"
#include "platform.cpp"
#include "input.cpp"
#include "renderer.cpp"
#include "sound.cpp"
#include "collision.cpp"
#include "entity.cpp"
#include "random.cpp"

enum gamestate
{
    State_Initial,
    State_Game,
    State_Pause,
    State_GameOver,
};

// Application Variables
global u32 WindowWidth = 1366;
global u32 WindowHeight = 768;
global b32 IsRunning;
global keyboard     *Keyboard;
global mouse        *Mouse;
global clock        *Clock;
global window       *Window;
global renderer     *Renderer;
global sound_system *SoundSystem;
global camera       *Camera;
global gamestate     CurrentState;

// Textures
global texture *InitScreenTexture  = NULL;
global texture *PauseScreenTexture = NULL;
global texture *GameOverTexture    = NULL;
global texture *PlayerTexture      = NULL;
global texture *BackgroundTexture  = NULL;
global texture *WallTexture        = NULL;
global texture *BulletTexture      = NULL;
global texture *PointerTexture     = NULL;
global texture *WandererTexture    = NULL;
global texture *SeekerTexture      = NULL;
global texture *KamikazeTexture    = NULL;

// Fonts
font *DebugFont = NULL;
font *UIFont = NULL;

// Sounds/Music
global sound_music  *Song = NULL;
global sound_effect *Shot = NULL;
global sound_effect *PlayerDeath = NULL;
global sound_effect *PlayerDamage = NULL;

// Game Variables
global f32 WorldBottom      = -11.0f;
global f32 WorldTop         = 11.0f;
global f32 WorldLeft        = -20.0f;
global f32 WorldRight       = 20.0f;
global f32 HalfWorldWidth   = WorldRight;
global f32 HalfWorldHeight  = WorldTop;
global f32 WorldWidth       = WorldRight * 2.0f;
global f32 WorldHeight      = WorldTop * 2.0f;
global f32 BackgroundWidth  = WorldWidth + 5.0f;
global f32 BackgroundHeight = WorldHeight + 5.0f;
global u32 PlayerScore      = 0;
global b32 DebugMode        = 0;
global i32 EnemySpawnRate   = 50;
global f32 BulletSpeed      = 20.0f;
global entity *LeftWall     = NULL;
global entity *RightWall    = NULL;
global entity *TopWall      = NULL;
global entity *BottomWall   = NULL;

// Entities
entity *GameBackground = NULL;
entity *InitialScreen  = NULL;
entity *PauseScreen    = NULL;
entity *GameOverScreen = NULL;

// Bullet and enemies containers
u32 MaxEntityCount   = 100;
entity_list *Enemies = NULL;
entity_list *Bullets = NULL;

// Player vars
global entity *Player                  = NULL;
global f32 PlayerSpeed                 = 3.0f;
global f32 PlayerDrag                  = 0.8f;
global u32 PlayerLives                 = 10;
global glm::vec3 PlayerInitialPosition = glm::vec3(0.0f, 0.0f, 0.0f);

void SpawnSeeker(entity_list *List)
{
    f32 PosX = RandomBetween(WorldLeft, WorldRight);
    f32 PosY = RandomBetween(WorldBottom, WorldTop);
    entity *Entity = E_CreateEntity(SeekerTexture, glm::vec3(PosX, PosY, 0.0f), glm::vec3(0), glm::vec3(1.0f), 0.0f, 2.0f, 0.8f, EntityType_Seeker, Collider_Rectangle);
    E_PushEntity(List, Entity);
}

void SpawnWanderer(entity_list *List)
{
    f32 PosX = RandomBetween(WorldLeft, WorldRight);
    f32 PosY = RandomBetween(WorldBottom, WorldTop);
    entity *Entity = E_CreateEntity(WandererTexture, glm::vec3(PosX, PosY, 0.0f), glm::vec3(0), glm::vec3(1.0f), 0.0f, 0.0f, 1.0f, EntityType_Wanderer, Collider_Rectangle);
    E_PushEntity(List, Entity);
}

void SpawnKamikaze(entity_list *List)
{
    // Spawns an enemy that flies directly to the player position.

    f32 PosX = 0;
    f32 PosY = 0;
    f32 Padding = 3.0f;

    // Random PosX
    if(RandomBool())
    {
        // Left
        PosX = RandomBetween(WorldLeft - Padding, WorldLeft);
    }
    else
    {
        // Right
        PosX = RandomBetween(WorldRight, WorldRight + Padding);
    }

    // Random  PosY
    if(RandomBool())
    {
        // Top
        PosY = RandomBetween(WorldTop, WorldTop + Padding);
    }
    else
    {
        // Bottom
        PosY = RandomBetween(WorldBottom - Padding, WorldBottom);
    }

    glm::vec3 KamikazePosition = {PosX, PosY, 0.0f};
    f32 DirectionX = KamikazePosition.x - Player->Position.x;
    f32 DirectionY = KamikazePosition.y - Player->Position.y;
    f32 KamikazeAngle = GetDirectionAngle(DirectionY, DirectionX);
    f32 KamikazeSpeed = 5.0f;
    entity *Entity = E_CreateEntity(KamikazeTexture, KamikazePosition,
                                    Player->Position - KamikazePosition,
                                    glm::vec3(1.0f, 1.0f, 0.0f), KamikazeAngle, KamikazeSpeed, 0.8f,
                                    EntityType_Kamikaze, Collider_Rectangle);
    E_PushEntity(List, Entity);
}

void SpawnEnemies()
{
    if(RandomBetween(0, EnemySpawnRate) == 15)
    {
        SpawnSeeker(Enemies);
    }
    if(RandomBetween(0, EnemySpawnRate) == 16)
    {
        SpawnKamikaze(Enemies);
    }
    if(RandomBetween(0, EnemySpawnRate) == 17)
    {
        SpawnWanderer(Enemies);
    }
}

void ApplyEnemyInputs()
{
    // Enemy AI
    // Set "inputs" according to enemy type, i can't figure out a better place to put the enemy AI and i'm not gonna think too much about it
    for(entity_node *Node = Enemies->Head;
        Node != NULL;
        Node = Node->Next)
    {
        entity *Entity = Node->Entity;

        switch(Entity->Type)
        {
            case EntityType_Seeker:
            {
                // Get Angle to player, and move towards the player
                f32 DeltaX = Entity->Position.x - Player->Position.x;
                f32 DeltaY = Entity->Position.y - Player->Position.y;
                f32 RotationAngle = GetDirectionAngle(DeltaY, DeltaX);
                Entity->Angle = RotationAngle;
                glm::vec3 SeekerDirection = Direction(Entity->Position, Player->Position);
                Entity->Acceleration += SeekerDirection * Entity->Speed;
            } break;
            case EntityType_Wanderer:
            {
                f32 X = Cosf((f32)Clock->SecondsElapsed);
                f32 Y = Sinf((f32)Clock->SecondsElapsed);
                Entity->Position.x += X * (f32)Clock->DeltaTime * 3.0f;
                Entity->Position.y += Y * (f32)Clock->DeltaTime * 3.0f;
                Entity->Angle += 0.4f;
            } break;
            case EntityType_Kamikaze:
            {
                Entity->Acceleration += glm::normalize(Entity->Direction) * Entity->Speed;
                break;
            }
            case EntityType_Pickup:
            case EntityType_Bullet:
            case EntityType_None:
            case EntityType_Wall:
            case EntityType_Player:
            default:
            {
                InvalidCodePath;
                break;
            }
        }
    }

}

void UpdateEnemyPositions()
{
    for(entity_node *Node = Enemies->Head;
        Node != NULL;
        Node = Node->Next)
    {
        E_Update(Node->Entity, (f32)Clock->DeltaTime);

        if(Node->Entity->Type == EntityType_Kamikaze)
        {
            if(Magnitude(Node->Entity->Position) > 30.0f)
            {
                E_FreeNode(Enemies, Node);
            }
        }
    }
}

void UpdateBulletPositions()
{
    for(entity_node *Node = Bullets->Head;
        Node != NULL;
        Node = Node->Next)
    {
        E_Update(Node->Entity, (f32)Clock->DeltaTime);

        // If the bullet is no longer near the play
        // area, delete this. Maybe later just checked
        // square distances to avoid a sqrt.
        if(Magnitude(Node->Entity->Position) > 30.0f)
        {
            E_FreeNode(Bullets, Node);
        }
    }
}

void UpdatePlayerPosition()
{
    // Rotate player according to mouse world position
    f32 DeltaX = Player->Position.x - Mouse->WorldPosition.x;
    f32 DeltaY = Player->Position.y - Mouse->WorldPosition.y;
    Player->Angle = GetDirectionAngle(DeltaY, DeltaX);
    E_Update(Player, (f32)Clock->DeltaTime);
}

void FireBullet()
{
    f32 DeltaX = Player->Position.x - Mouse->WorldPosition.x;
    f32 DeltaY = Player->Position.y - Mouse->WorldPosition.y;
    f32 RotationAngle = GetDirectionAngle(DeltaY, DeltaX);
    glm::vec3 BulletDirection = glm::normalize(Mouse->WorldPosition - Player->Position);
    f32 ScalingFactor = 3.5f;
    entity *NewBullet = E_CreateEntity(BulletTexture, Player->Position, glm::vec3(0.0f), glm::vec3(0.31f * ScalingFactor, 0.11f * ScalingFactor, 0.0f), RotationAngle, BulletSpeed, 1.0f, EntityType_Bullet, Collider_Rectangle);
    NewBullet->Acceleration += BulletDirection * BulletSpeed;
    E_PushEntity(Bullets, NewBullet);
    S_PlaySoundEffect(Shot);
}

void UpdateWorldMousePosition()
{
    // Convert window mouse position to game world position, mainly used to fire bullets from Player->Position to Mouse->WorldPosition
    Mouse->WorldPosition.x = Remap((f32)(Mouse->X), 0.0f, (f32)Window->Width, Camera->Position.x - HalfWorldWidth, Camera->Position.x + HalfWorldWidth);
    Mouse->WorldPosition.y = Remap((f32)(Mouse->Y), 0.0f, (f32)Window->Height, Camera->Position.y + HalfWorldHeight, Camera->Position.y - HalfWorldHeight);
}

void CollisionPlayerVsWalls()
{
    glm::vec2 ResolutionDirection;
    f32 ResolutionOverlap;

    // Collision Player vs Walls
    if(E_EntitiesCollide(Player, LeftWall, &ResolutionDirection, &ResolutionOverlap))
    {
        glm::vec2 I = ResolutionDirection * ResolutionOverlap;
        Player->Position.x -= I.x;
        Player->Position.y -= I.y;
    }
    if(E_EntitiesCollide(Player, RightWall, &ResolutionDirection, &ResolutionOverlap))
    {
        glm::vec2 I = ResolutionDirection * ResolutionOverlap;
        Player->Position.x -= I.x;
        Player->Position.y -= I.y;
    }
    if(E_EntitiesCollide(Player, TopWall, &ResolutionDirection, &ResolutionOverlap))
    {
        glm::vec2 I = ResolutionDirection * ResolutionOverlap;
        Player->Position.x -= I.x;
        Player->Position.y -= I.y;
    }
    if(E_EntitiesCollide(Player, BottomWall, &ResolutionDirection, &ResolutionOverlap))
    {
        glm::vec2 I = ResolutionDirection * ResolutionOverlap;
        Player->Position.x -= I.x;
        Player->Position.y -= I.y;
    }
}

void CollisionPlayerVsEnemies()
{
    glm::vec2 ResolutionDirection;
    f32 ResolutionOverlap;

    // Collision Player vs Enemies
    for(entity_node *Node = Enemies->Head;
        Node != NULL;
        Node = Node->Next)
    {
        if(E_EntitiesCollide(Player, Node->Entity, &ResolutionDirection, &ResolutionOverlap))
        {
            // Player got hit, play dmg sound effect, check if dead
            E_FreeNode(Enemies, Node);
            PlayerLives--;

            if(PlayerLives < 1)
            {
                S_PlaySoundEffect(PlayerDeath);
                EnableBloom = 0;
                CurrentState = State_GameOver;
                R_ResetCamera(Camera, Window->Width, Window->Height, glm::vec3(0.0f, 0.0f, 11.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            else
            {
                S_PlaySoundEffect(PlayerDamage);
            }

        }
    }
}

void CollisionEnemiesVsBullets()
{
    glm::vec2 ResolutionDirection;
    f32 ResolutionOverlap;

    // Enemies vs Player Bullets,  note: this is a n*m loop
    for(entity_node *Enemy = Enemies->Head;
        Enemy != NULL;
        Enemy = Enemy->Next)
    {
        for(entity_node *Bullet = Bullets->Head;
            Bullet != NULL;
            Bullet = Bullet->Next)
        {
            if(E_EntitiesCollide(Enemy->Entity, Bullet->Entity, &ResolutionDirection, &ResolutionOverlap))
            {
                PlayerScore += 1;
                E_FreeNode(Enemies, Enemy);
                E_FreeNode(Bullets, Bullet);
            }
        }
    }
}

void GameBegin()
{
    IsRunning = 1;
    CurrentState = State_Initial;
    Enemies = E_CreateEntityList(MaxEntityCount);
    Bullets = E_CreateEntityList(MaxEntityCount);
    S_PlayMusic(Song);
}

i32 main(i32 Argc, char **Argv)
{
    // The following makes the compiler not throw a warning for unused
    // variables. I don't really want to turn that warning off, so
    // here it is, a line of nonsense.
    Argc; Argv;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

    Window       = P_CreateOpenGLWindow("Glow", WindowWidth, WindowHeight);
    Renderer     = R_CreateRenderer(Window);
    Keyboard     = I_CreateKeyboard();
    Mouse        = I_CreateMouse();
    Clock        = P_CreateClock();
    SoundSystem  = S_CreateSoundSystem();
    Camera       = R_CreateCamera(Window->Width, Window->Height, glm::vec3(0.0f, 0.0f, 11.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // NOTE(Jorge): Seed the RNG, GetPerformanceCounter is not the
    // best way, but the results look acceptable. I do not have a CPU
    // that supports RDSEED at the moment. We can probably make
    // another RNG instance, seed it with performance counter and use
    // the next random number as the seed for the RNG we really are
    // going to use.
    RandomSeed((u32)SDL_GetPerformanceCounter());

    // Load Fonts
    DebugFont = R_CreateFont(Renderer, "fonts/LiberationMono-Regular.ttf", 14, 14);
    UIFont    = R_CreateFont(Renderer, "fonts/NovaSquare-Regular.ttf", 30, 30);

    // Load music and sounds
    Song         = S_CreateMusic("audio/Music.mp3");
    Shot         = S_CreateSoundEffect("audio/Shoot1.wav");
    PlayerDeath  = S_CreateSoundEffect("audio/Explosion1.wav");
    PlayerDamage = S_CreateSoundEffect("audio/Explosion6.wav");

    // Load textures
    InitScreenTexture  = R_CreateTexture("textures/InitialScreen.png");
    PauseScreenTexture = R_CreateTexture("textures/PauseScreen.png");
    GameOverTexture    = R_CreateTexture("textures/GameOver.png");
    PlayerTexture      = R_CreateTexture("textures/Player.png");
    BackgroundTexture  = R_CreateTexture("textures/DeepBlue.png");
    WallTexture        = R_CreateTexture("textures/Yellow.png");
    BulletTexture      = R_CreateTexture("textures/Bullet.png");
    PointerTexture     = R_CreateTexture("textures/Pointer.png");
    WandererTexture    = R_CreateTexture("textures/Wanderer.png");
    SeekerTexture      = R_CreateTexture("textures/Seeker.png");
    KamikazeTexture    = R_CreateTexture("textures/Kamikaze.png");

    // Create Game Entities
    InitialScreen  = E_CreateEntity(InitScreenTexture, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0), glm::vec3(BackgroundWidth, BackgroundHeight, 0.0f), 0.0f, 0.0f, 0.0f, EntityType_None, Collider_Rectangle);
    PauseScreen    = E_CreateEntity(PauseScreenTexture, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0), glm::vec3(BackgroundWidth, BackgroundHeight, 0.0f), 0.0f, 0.0f, 0.0f, EntityType_None, Collider_Rectangle);
    GameOverScreen = E_CreateEntity(GameOverTexture, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0), glm::vec3(BackgroundWidth, BackgroundHeight, 0.0f), 0.0f, 0.0f, 0.0f, EntityType_None, Collider_Rectangle);
    GameBackground = E_CreateEntity(BackgroundTexture, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0), glm::vec3(BackgroundWidth, BackgroundHeight, 0.0f), 0.0f, 0.0f, 0.0f, EntityType_None, Collider_Rectangle);
    LeftWall       = E_CreateEntity(WallTexture, glm::vec3(WorldLeft - 1.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f, BackgroundHeight, 0.0f), 0.0f, 0.0f, 0.0f, EntityType_Wall, Collider_Rectangle);
    RightWall      = E_CreateEntity(WallTexture, glm::vec3(WorldRight + 1.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f, BackgroundHeight, 0.0f), 0.0f, 0.0f, 0.0f, EntityType_Wall, Collider_Rectangle);
    TopWall        = E_CreateEntity(WallTexture, glm::vec3(0.0f, WorldTop + 1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(BackgroundWidth, 1.0f, 0.0f), 0.0f, 0.0f, 0.0f, EntityType_Wall, Collider_Rectangle);
    BottomWall     = E_CreateEntity(WallTexture, glm::vec3(0.0f, WorldBottom - 1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(BackgroundWidth, 1.0f, 0.0f), 0.0f, 0.0f, 0.0f, EntityType_Wall, Collider_Rectangle);
    Player         = E_CreateEntity(PlayerTexture, PlayerInitialPosition, glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 0.0f), 0.0f, PlayerSpeed, PlayerDrag, EntityType_Player, Collider_Circle);

    GameBegin();
    while(IsRunning)
    {
        P_UpdateClock(Clock);
        R_CalculateFPS(Renderer, Clock);

        { // SECTION: Input Handling
            SDL_Event Event;
            while (SDL_PollEvent(&Event))
            {
                switch (Event.type)
                {
                    case SDL_QUIT:
                    {
                        IsRunning = 0;
                        break;
                    }
                    case SDL_WINDOWEVENT:
                    {
                        if (Event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                        {
                            CurrentState = State_Pause;
                        }
                    }
                }
            }

            I_UpdateKeyboard(Keyboard);
            I_UpdateMouse(Mouse);

            UpdateWorldMousePosition();

            switch (CurrentState)
            {
                case State_Initial:
                {
                    EnableBloom = 0;
                    if (I_IsPressed(SDL_SCANCODE_ESCAPE)) { IsRunning = 0; }
                    if (I_IsPressed(SDL_SCANCODE_SPACE))  { CurrentState = State_Game; EnableBloom = 1; }
                    break;
                }
                case State_Game:
                {
                    // Explore/Screenshot mode
                    // if(I_IsPressed(SDL_SCANCODE_Z))
                    // {
                    //     Here!;
                    // }

                    // Press escape to pause game
                    if (I_IsPressed(SDL_SCANCODE_ESCAPE))
                    {
                        CurrentState = State_Pause;
                        S_PauseMusic();
                        EnableBloom = 0;
                    }

                    // F1 -> Draw Debug Info Toggle
                    if(I_IsPressed(SDL_SCANCODE_F1) && I_WasNotPressed(SDL_SCANCODE_F1))
                    {
                        // Reset camera when disabling debug mode
                        if(DebugMode) { R_ResetCamera(Camera, Window->Width, Window->Height, glm::vec3(0.0f, 0.0f, 11.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)); I_ResetMouse(Mouse); }
                        DebugMode = !DebugMode;
                    }

                    // Camera Movement, this only applies when Debug
                    // Info is shown. press shift + WASD + Mouse to
                    // move the camera. Shift+Space to reset the
                    // camera.
                    if (I_IsPressed(SDL_SCANCODE_LSHIFT) & DebugMode)
                    {
                        Camera->Yaw += Mouse->RelX * Mouse->Sensitivity;
                        Camera->Pitch += -Mouse->RelY * Mouse->Sensitivity; // reversed since y-coordinates range from bottom to top

                        if (Camera->Pitch > 89.0f)
                        {
                            Camera->Pitch = 89.0f;
                        }
                        else if (Camera->Pitch < -89.0f)
                        {
                            Camera->Pitch = -89.0f;
                        }
                        glm::vec3 Front;
                        Front.x = cos(glm::radians(Camera->Yaw)) * cos(glm::radians(Camera->Pitch));
                        Front.y = sin(glm::radians(Camera->Pitch));
                        Front.z = sin(glm::radians(Camera->Yaw)) * cos(glm::radians(Camera->Pitch));
                        Camera->Front = glm::normalize(Front);

                        if (I_IsPressed(SDL_SCANCODE_W) && I_IsPressed(SDL_SCANCODE_LSHIFT)) { Camera->Position += Camera->Front * Camera->Speed * (f32)Clock->DeltaTime; }
                        if (I_IsPressed(SDL_SCANCODE_S) && I_IsPressed(SDL_SCANCODE_LSHIFT)) { Camera->Position -= Camera->Speed * Camera->Front * (f32)Clock->DeltaTime; }
                        if (I_IsPressed(SDL_SCANCODE_A) && I_IsPressed(SDL_SCANCODE_LSHIFT)) { Camera->Position -= glm::normalize(glm::cross(Camera->Front, Camera->Up)) * Camera->Speed * (f32)Clock->DeltaTime; }
                        if (I_IsPressed(SDL_SCANCODE_D) && I_IsPressed(SDL_SCANCODE_LSHIFT)) { Camera->Position += glm::normalize(glm::cross(Camera->Front, Camera->Up)) * Camera->Speed * (f32)Clock->DeltaTime; }
                        if (I_IsPressed(SDL_SCANCODE_SPACE) && I_IsPressed(SDL_SCANCODE_LSHIFT)) { R_ResetCamera(Camera, Window->Width, Window->Height, glm::vec3(0.0f, 0.0f, 11.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)); I_ResetMouse(Mouse); }
                    }

                    // Player Input and movement
                    if (I_IsPressed(SDL_SCANCODE_W) && I_IsNotPressed(SDL_SCANCODE_LSHIFT)) { Player->Acceleration.y += Player->Speed; }
                    if (I_IsPressed(SDL_SCANCODE_A) && I_IsNotPressed(SDL_SCANCODE_LSHIFT)) { Player->Acceleration.x -= Player->Speed; }
                    if (I_IsPressed(SDL_SCANCODE_S) && I_IsNotPressed(SDL_SCANCODE_LSHIFT)) { Player->Acceleration.y -= Player->Speed; }
                    if (I_IsPressed(SDL_SCANCODE_D) && I_IsNotPressed(SDL_SCANCODE_LSHIFT)) { Player->Acceleration.x += Player->Speed; }

                    // Fire Bullet on Mouse Button Left press
                    if(I_IsMouseButtonPressed(SDL_BUTTON_LEFT) && I_WasMouseButtonNotPressed(SDL_BUTTON_LEFT)) { FireBullet(); }

                    ApplyEnemyInputs();

                    break;
                }
                case State_Pause:
                {
                    if (I_IsPressed(SDL_SCANCODE_ESCAPE) && I_WasNotPressed(SDL_SCANCODE_ESCAPE)) { IsRunning = 0; }
                    if (I_IsPressed(SDL_SCANCODE_SPACE) && I_WasNotPressed(SDL_SCANCODE_SPACE))
                    {
                        CurrentState = State_Game;
                        S_ResumeMusic();
                        R_ResetCamera(Camera, Window->Width, Window->Height, glm::vec3(0.0f, 0.0f, 11.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                        EnableBloom = 1;
                    }

                    break;
                }
                case State_GameOver:
                {
                    if (I_IsPressed(SDL_SCANCODE_ESCAPE) && I_WasNotPressed(SDL_SCANCODE_ESCAPE)) { IsRunning = 0; }

                    if (I_IsPressed(SDL_SCANCODE_SPACE) && I_WasNotPressed(SDL_SCANCODE_SPACE))
                    {
                        PlayerScore = 0;
                        CurrentState = State_Game;
                        E_EmptyList(Enemies);
                        E_EmptyList(Bullets);
                        Player->Position = PlayerInitialPosition;
                        PlayerLives = 3;
                        EnableBloom = 1;
                        R_ResetCamera(Camera, Window->Width, Window->Height, glm::vec3(0.0f, 0.0f, 11.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    break;
                }
                default:
                {
                    // Invalid code path
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", "InputHandling invalid code path: default", Window->Handle);
                    exit(0);
                    break;
                }
            }

            // Toggle fullscreen in every Game_State, that's why its outside of the switch statement
            if (I_IsReleased(SDL_SCANCODE_RETURN) && I_IsPressed(SDL_SCANCODE_LALT))
            {
                P_ToggleFullscreen(Window);
                R_ResizeRenderer(Renderer, Window->Width, Window->Height);
            }

        } // SECTION END: Input Handling

        { // SECTION: Update
            switch(CurrentState)
            {
                case State_Initial:
                {
                    break;
                }
                case State_Game:
                {
                    SpawnEnemies();

                    UpdatePlayerPosition();
                    UpdateEnemyPositions();
                    UpdateBulletPositions();

                    CollisionPlayerVsWalls();
                    CollisionPlayerVsEnemies();
                    CollisionEnemiesVsBullets();

                    break;
                }
                case State_Pause:
                {
                    break;
                }
                case State_GameOver:
                {
                    break;
                }
                default:
                {
                    // Invalid code path
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", "UpdateState invalid code path: default", Window->Handle);
                    exit(0);
                    break;
                }
            }

            R_UpdateCamera(Renderer, Camera);

        } // SECTION END: Update

        { // SECTION: Render
            R_BeginFrame(Renderer);

            switch(CurrentState)
            {
                case State_Initial:
                {
                    Renderer->BackgroundColor = MenuBackgroundColor;
                    R_SetActiveShader(Renderer->Shaders.Texture);
                    R_DrawEntity(Renderer, InitialScreen);
                    break;
                }
                case State_Game:
                {
                    Renderer->BackgroundColor = BackgroundColor;

                    R_SetActiveShader(Renderer->Shaders.Texture);

                    R_DrawEntity(Renderer, GameBackground);
                    R_DrawEntity(Renderer, Player);

                    R_DrawEntityList(Renderer, Enemies);
                    R_DrawEntityList(Renderer, Bullets);

                    // Draw Mouse Pointer. The Position needs
                    // adjustment since R_DrawTexture draws
                    // centered. Could also create a crosshair image
                    // and it would be perfect.
                    glm::vec3 CursorSize = glm::vec3(0.44f, 0.62f, 0.0f);
                    glm::vec3 CorrectedCursorPosition = glm::vec3(Mouse->WorldPosition.x - (CursorSize.x / 2.0f),
                                                                  Mouse->WorldPosition.y - (CursorSize.y / 2.0f),
                                                                  0.1f);
                    R_DrawTexture(Renderer, PointerTexture, CorrectedCursorPosition, CursorSize, glm::vec3(0.0f), 0.0f);

                    // Draw player score
                    char StringBuffer[80];
                    sprintf_s(StringBuffer, "Score: %d", PlayerScore);
                    R_DrawText2D(Renderer, StringBuffer, UIFont, glm::vec2(Window->Width - UIFont->Width * 5 , Window->Height-UIFont->Height * 2), glm::vec2(1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

                    // Draw player Lives
                    sprintf_s(StringBuffer, "Lives: %d", PlayerLives);
                    R_DrawText2D(Renderer, StringBuffer, UIFont, glm::vec2(Window->Width - UIFont->Width * 5 , Window->Height-UIFont->Height), glm::vec2(1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

                    // Draw debug data to the screen, the current text
                    // rendering implementation sucks ass, so while
                    // rendering the debug data the frame tanks unless
                    // the computer is powerful.
                    if(DebugMode)
                    {
                        // String buffer used for snprintf
                        char String[100] = {};

                        // GPU and OpenGL stuff
                        f32 LeftMargin = 4.0f;
                        R_DrawText2D(Renderer, "GPU:", DebugFont, glm::vec2(LeftMargin, Window->Height - DebugFont->Height), glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
                        R_DrawText2D(Renderer, (char*)Renderer->HardwareVendor, DebugFont, glm::vec2(LeftMargin * 2, Window->Height - DebugFont->Height * 2), glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
                        R_DrawText2D(Renderer, (char*)Renderer->HardwareModel, DebugFont, glm::vec2(LeftMargin * 2, Window->Height - DebugFont->Height * 3), glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
                        snprintf(String, sizeof(char) * 99,"OpenGL Version: %s", Renderer->OpenGLVersion);
                        R_DrawText2D(Renderer, String, DebugFont, glm::vec2(LeftMargin * 2, Window->Height - DebugFont->Height * 4), glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
                        snprintf(String, sizeof(char) * 99,"GLSL Version: %s", Renderer->GLSLVersion);
                        R_DrawText2D(Renderer, String, DebugFont, glm::vec2(LeftMargin * 2, Window->Height - DebugFont->Height * 5), glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

                        // CPU
                        R_DrawText2D(Renderer, "CPU:", DebugFont, glm::vec2(LeftMargin, Window->Height - DebugFont->Height * 6), glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
                        snprintf(String, sizeof(char) * 99,"Cache Line Size: %d", SDL_GetCPUCacheLineSize());
                        R_DrawText2D(Renderer, String, DebugFont, glm::vec2(LeftMargin * 2, Window->Height - DebugFont->Height * 7), glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
                        snprintf(String, sizeof(char) * 99,"Core Count: %d", SDL_GetCPUCount());
                        R_DrawText2D(Renderer, String, DebugFont, glm::vec2(LeftMargin * 2, Window->Height - DebugFont->Height * 8), glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

                        // FPS Min Ms/Max Ms/ Avg Ms
                        snprintf(String, sizeof(char) * 99,"FPS: %.4f", Renderer->FPS);
                        R_DrawText2D(Renderer, String, DebugFont, glm::vec2(LeftMargin, Window->Height - DebugFont->Height * 9), glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
                        snprintf(String, sizeof(char) * 99,"Average Ms Per Frame: %.5f", Renderer->AverageMsPerFrame);
                        R_DrawText2D(Renderer, String, DebugFont, glm::vec2(LeftMargin, Window->Height - DebugFont->Height * 10), glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

                        // Player World Position
                        snprintf(String, sizeof(char) * 99,"Player->WorldPosition: X:%.2f Y:%.2f", Player->Position.x, Player->Position.y);
                        R_DrawText2D(Renderer, String, DebugFont, glm::vec2(LeftMargin, Window->Height - DebugFont->Height * 11), glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

                        // Mouse World Position
                        snprintf(String, sizeof(char) * 99,"Mouse->WorldPosition: X:%.2f Y:%.2f", Mouse->WorldPosition.x, Mouse->WorldPosition.y);
                        R_DrawText2D(Renderer, String, DebugFont, glm::vec2(LeftMargin, Window->Height - DebugFont->Height * 12), glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

                        // Entity Count
                        snprintf(String, sizeof(char) * 99,"EntityCount: %d", Enemies->Count + Bullets->Count + 1); // The + 1 means the player
                        R_DrawText2D(Renderer, String, DebugFont, glm::vec2(LeftMargin, Window->Height - DebugFont->Height * 13), glm::vec2(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
                    }

                    break;
                }
                case State_Pause:
                {
                    Renderer->BackgroundColor = MenuBackgroundColor;
                    R_SetActiveShader(Renderer->Shaders.Texture);
                    R_DrawEntity(Renderer, PauseScreen);

                    break;
                }
                case State_GameOver:
                {
                    Renderer->BackgroundColor = MenuBackgroundColor;
                    R_SetActiveShader(Renderer->Shaders.Texture);
                    R_DrawEntity(Renderer, GameOverScreen);
                    break;
                }
                default:
                {
                    InvalidCodePath;
                    break;
                }
            }

            R_EndFrame(Renderer);
        } // SECTION END: Render
    }

    SDL_GL_DeleteContext(Window->Handle);

    return 0;
}
