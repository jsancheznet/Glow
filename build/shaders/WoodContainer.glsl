#ifdef VERTEX_SHADER

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 UV;


uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec3 NormalVector;
out vec3 FragPos;
out vec2 TexCoords;

void main()
{
    // NormalVector = Normal;
    // NOTE: To transfor normal vectors to world space we need this funky transpose/inverse/3x3 matrix math.
    NormalVector = mat3(transpose(inverse(Model))) * Normal;
    FragPos = vec3(Model * vec4(Position, 1.0));
    TexCoords = UV;
	gl_Position = Projection * View * Model * vec4(Position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct material
{
    sampler2D Diffuse;
    sampler2D Specular;
    float Shininess;
};

struct directional_light
{
    vec3 Direction;
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
};

struct point_light
{
    vec3 Position;

    float Constant;
    float Linear;
    float Quadratic;

    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
};

#define NR_POINT_LIGHTS 4
uniform point_light PointLights[NR_POINT_LIGHTS];
uniform directional_light DirLight;
uniform vec3 ViewPos;
uniform material Material;

in vec3 NormalVector;
in vec3 FragPos;
in vec2 TexCoords;

vec3 CalcPointLight(point_light Light, vec3 Normal, vec3 FragPos, vec3 ViewDir)
{
    vec3 LightDir = normalize(Light.Position - FragPos);
    // Diffuse shading
    float Diff = max(dot(Normal, LightDir), 0.0);
    // Specular Shading
    vec3 ReflectDir = reflect(-LightDir, Normal);
    float Spec = pow(max(dot(ViewDir, ReflectDir), 0.0), Material.Shininess);
    // Attenuation
    float Distance = length(Light.Position - FragPos);
    float Attenuation = 1.0 / (Light.Constant + Light.Linear * Distance + Light.Quadratic * (Distance * Distance));
    // Combine Results
    vec3 Ambient = Light.Ambient * vec3(texture(Material.Diffuse, TexCoords));
    vec3 Diffuse = Light.Diffuse * Diff * vec3(texture(Material.Diffuse, TexCoords));
    vec3 Specular = Light.Specular * Spec * vec3(texture(Material.Specular, TexCoords));
    Ambient *= Attenuation;
    Diffuse *= Attenuation;
    Specular *= Attenuation;

    return (Ambient + Diffuse + Specular);
}

vec3
CalcDirLight(directional_light Light, vec3 Normal, vec3 ViewDirection)
{
    vec3 LightDir = normalize(-Light.Direction);
    // Diffuse Shading
    float Diff = max(dot(Normal, LightDir), 0.0);
    // Specular shading
    vec3 ReflectDir = reflect(-LightDir, Normal);
    float Spec = pow(max(dot(ViewDirection, ReflectDir), 0.0), Material.Shininess);
    // Combine Results
    vec3 Ambient = Light.Ambient * vec3(texture(Material.Diffuse, TexCoords));
    vec3 Diffuse = Light.Diffuse * Diff * vec3(texture(Material.Diffuse, TexCoords));
    vec3 Specular = Light.Specular * Spec * vec3(texture(Material.Specular, TexCoords));
    return (Ambient + Diffuse + Specular);
}

void main()
{
    // Properties
    vec3 Norm = normalize(NormalVector);
    vec3 ViewDir = normalize(ViewPos - FragPos);

    // Phase 1: Directional Lighting
    vec3 Result = CalcDirLight(DirLight, Norm, ViewDir);
    // Phase 2: Point Lights
    for(int i = 0; i  < NR_POINT_LIGHTS; i++)
    {
        Result += CalcPointLight(PointLights[i], Norm, FragPos, ViewDir);
    }

    // Phase 3: Spot Light
    // Result += CalcSpotLight(SpotLight, Norm, FragPos, ViewDir);

    FragColor = vec4(Result, 1.0);

    // Check wether fragment output is higher than threshol, if so, output as brightness color
    float Brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(Brightness > 1.0)
    {
        BrightColor = vec4(FragColor.rgb, 1.0);
    }
    else
    {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
#endif
