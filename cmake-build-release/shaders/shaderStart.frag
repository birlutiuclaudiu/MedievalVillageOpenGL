#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 directionalLightDir;  //directional
uniform vec3 directionalLightColor;
uniform vec3 pointLightDir;   //point
uniform vec3 pointLightColor;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//components
float ambientStrength = 0.1f;
vec3 ambientDir;     vec3 ambientPoint;
vec3 diffuseDir;     vec3 diffusePoint;
vec3 specularDir;    vec3 specularPoint;
float specularStrength = 0.5f;

//pentru umbra
in vec4 fragPosLightSpace;
//pentru ceata
uniform int isFog;

void computeDirLight()
{
	//compute eye space coordinates
	vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
	vec3 normalEye = normalize(normalMatrix * fNormal);

	//normalize light direction
	vec3 lightDirN = vec3(normalize(view *vec4(directionalLightDir, 0.0f)));

	//compute view direction (in eye coordinates, the viewer is situated at the origin
	vec3 viewDir = normalize(- fPosEye.xyz);
	//compute ambient light
	ambientDir = ambientStrength * directionalLightColor;
	vec3 halfVector = normalize(lightDirN+viewDir);
	//compute diffuse light
	diffuseDir = max(dot(normalEye, lightDirN), 0.0f) * directionalLightColor;

	//compute specular light
	//vec3 reflectDir = reflect(lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), 32);
	specularDir = specularStrength * specCoeff * directionalLightColor;
}

void computePointLight(){
	//compute light direction
	vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
	vec3 normalEye = normalize(normalMatrix * fNormal);
	vec3 viewDir = normalize(- fPosEye.xyz);   //view-url situat in orgine
	vec3 lightPosEye = vec3(view*vec4(pointLightDir, 1.0f));
	vec3 lightDirN = normalize(lightPosEye - fPosEye.xyz);

	float constant =1.0f;
	float linear = 0.07;
	float quadratic = 0.017;

	float dist = length(lightPosEye - fPosEye.xyz);
	//compute attenuation
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
	ambientPoint = att * (ambientStrength+0.2)* pointLightColor;
	diffusePoint = att* max(dot(normalEye, lightDirN), 0.0f) * pointLightColor;

	vec3 reflectDir = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
	specularPoint =  att* specularStrength * specCoeff * pointLightColor;

}

float computeShadow(){
	//perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	if (normalizedCoords.z > 200.0f)
		return 0.0f;
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
	float bias = max(0.05f* (1.0f-dot(fNormal, directionalLightDir)), 0.005f);
	float shadow = currentDepth -bias > closestDepth  ? 1.0f: 0.0f;
	return shadow;
}
float computeFog(){
	float fogDensity = 0.004f;
	float fragmentDistance = length(view * model * vec4(fPosition, 1.0f));
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
	return clamp(fogFactor, 0.0f, 1.0f);
}

void main()
{
	computeDirLight();
	computePointLight();
	vec3 ambient  = ambientDir  + ambientPoint;
	vec3 diffuse  = diffuseDir  + diffusePoint;
	vec3 specular = specularDir + specularPoint;
	float shadow = computeShadow();
	vec3 color = min((ambient + (1.0f -shadow)*diffuse)* texture(diffuseTexture, fTexCoords).rgb +
									(1.0f -shadow)*specular*texture(specularTexture, fTexCoords).rgb, 1.0f);

	vec4 color4 =  vec4(color, 1.0f);
	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	if (isFog==-1)
		color4 = fogColor * (1.0 - fogFactor) + color4* fogFactor;
	if (texture(diffuseTexture, fTexCoords).a < 0.1f)   //fragment discarding
		discard;
	else
		fColor = color4;
}
