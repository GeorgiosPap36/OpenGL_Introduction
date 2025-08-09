#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 1

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

in vec4 FragPosLightSpace; 

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;
uniform bool useSolidColor;
uniform vec3 solidColor;

uniform sampler2D shadowMap;



vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float DirectLightShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);

void main()
{    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 dirLightColor = CalcDirLight(dirLight, norm, viewDir);
    float shadow = DirectLightShadowCalculation(FragPosLightSpace, norm, normalize(-dirLight.direction));
    
    vec3 pointLightsColor = vec3(0.0);
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        pointLightsColor += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    vec3 result = (1.0 - shadow) * dirLightColor + pointLightsColor;

    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    vec3 ambient, diffuse, specular;
    if (useSolidColor) {
        ambient = light.ambient * solidColor;
        diffuse = light.diffuse * diff * solidColor;
        specular = light.specular * spec * solidColor;
    } else {
        ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    }
    
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    vec3 ambient, diffuse, specular;
    if (useSolidColor) {
        ambient = light.ambient * solidColor;
        diffuse = light.diffuse * diff * solidColor;
        specular = light.specular * spec * solidColor;
    } else {
        ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    }
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

float DirectLightShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check if coordinates are valid
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;
    }

    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    // float closestDepth = texture(shadowMap, projCoords.xy).r;
    // float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }    
    }
    shadow /= 9.0;

    return shadow;
}