#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
uniform float al;

//lighting
uniform vec3 viewPosEye;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 lightColorP;
uniform vec3 lightPos;
uniform vec3 lightPos1;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

uniform float constant;
uniform float linear;
uniform float quadratic;
uniform float fogDensity;

//components
 vec3 ambient;
vec3 diffuse;
vec3 specular;
float ambientStrength = 1.5f;
float difStrength = 3.0f;
float specularStrength = 50.0f;

vec3 ambientp;
vec3 diffusep;
vec3 specularp;
vec3 ambientp1;
vec3 diffusep1;
vec3 specularp1;
float ambientStrengthp = 50.0f;
float specularStrengthp = 1.0f;
float difStrengthp = 60.0f;
float shadow;

float computeFog()
{
 vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
 float fragmentDistance = length(fPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
 return clamp(fogFactor, 0.0f, 1.0f);
}

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));
    vec3 viewDir =normalize(viewPosEye - fPosEye.xyz);
    vec3 viewDirN = vec3(normalize(view * vec4(viewDir, 0.0f)));
    vec3 halfVector = normalize(lightDirN + viewDirN);
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), 320); 
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    
    ambient = ambientStrength * lightColor;
    diffuse = difStrength * max(dot(normalEye, lightDirN), 0.0f) * lightColor;
    specular = specularStrength * specCoeff * lightColor;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////pozitionala1

    vec4 lightPosEye = view * model * vec4(lightPos, 1.0f);
    float dist = length(lightPosEye.xyz - fPosEye.xyz);
    float dist2 = length(vec3(0.0, 0.0, 0.0) - fPosEye.xyz);
    float att1 = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
    float att2 = 1.0f / (40*constant + 30*linear * dist2 + 200000000*quadratic * (dist2 * dist2));
    float att = att1 + att2;
    vec3 lightDirNpoz = normalize(lightPosEye.xyz - fPosEye.xyz);
    
    vec3 halfVectp = normalize(lightDirNpoz + viewDirN);
    float specCoeffp = pow(max(dot(normalEye, halfVectp), 0.0f), 320); 

    ambientp = att * ambientStrengthp * lightColorP;
    diffusep = att * difStrengthp* max(dot(normalEye, lightDirNpoz), 0.0f) * lightColorP;
    specularp = att * specularStrengthp * specCoeffp * lightColorP;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////pozitionala2
// vec4 lightPosEye1 = view * model * vec4(lightPos1, 1.0f);
  //  float dist1 = length(lightPosEye1.xyz - fPosEye.xyz);
    //float att1 = 1.0f / (constant + linear * dist1 + quadratic * (dist1 * dist1));
//    vec3 lightDirNpoz1 = normalize(lightPosEye1.xyz - fPosEye.xyz);
    
  //  vec3 halfVectp1 = normalize(lightDirNpoz1 + viewDirN);
    //float specCoeffp1 = pow(max(dot(normalEye, halfVectp1), 0.0f), 320); 
// ambientp1 = att1 * ambientStrengthp * lightColorP;
  //  diffusep1 = att1 * max(dot(normalEye, lightDirNpoz1), 0.0f) * lightColorP;
    //specularp1 = att1 * specularStrengthp * specCoeffp1 * lightColorP;

}

void computeShadow()
{
    vec3 coords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    coords = coords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, coords.xy).r; 
    float currentDepth = coords.z;
    float bias= currentDepth -0.005f;
    shadow = bias > closestDepth  ? 1.0 : 0.0;
    if(coords.z > 1.0f) {
        shadow = 0.0f;
    }
}

void main() 
{   
    computeDirLight();
    computeShadow();
    float fogFactor = computeFog();
    vec4 fogColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);
//vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
   vec4 color = vec4(min(((ambient+ambientp)/2 + (1.0f - shadow)*(diffuse+diffusep)/2) * texture(diffuseTexture, fTexCoords).rgb +  (1.0f - shadow)*(specular+specularp)/2 * texture(specularTexture, fTexCoords).rgb, 1.0f),1.0f);   
fColor = fogColor*(1-fogFactor)+color*fogFactor;
fColor.w = al;
    
}
