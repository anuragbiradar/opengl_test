#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec3 objColor;
in vec3 lightPosition0;
in vec3 lightPosition1;
in vec3 lightColor0;

uniform float diffuseStrength;
uniform float specularStrength;

vec3 calLightPosition0()
{
	float ambientStrength = 0.1;
    	vec3 ambient = ambientStrength * lightColor0;

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPosition0 - FragPos);
	//vec3 lightDir = normalize(lightPosition0);
	float diff = max(dot(norm, lightDir), 1.0);
	vec3 diffuse = diff * diffuseStrength * lightColor0;

	vec3 viewDir = normalize(vec3(10.0f, 10.0f, -50.0f) - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 5);
	vec3 specular = specularStrength * spec * lightColor0;

	//Attenuation
	float distance = length(FragPos - lightPosition0);
	//constant linear quadratic
	float attenuation = 1.f / (1.f + 0.045f * distance + 0.0075f * (distance * distance));

	//Final light
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec3 calLightPosition1()
{
	float ambientStrength = 0.1;
    	vec3 ambient = ambientStrength * lightColor0;

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPosition1 - FragPos);
	//vec3 lightDir = normalize(lightPosition1);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * diffuseStrength * lightColor0;

	vec3 viewDir = normalize(vec3(10.0f, 10.0f, -50.0f) - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 5);
	vec3 specular = specularStrength * spec * lightColor0;

	//Attenuation
	float distance = length(FragPos - lightPosition1);
	//constant linear quadratic
	float attenuation = 1.f / (1.f + 0.045f * distance + 0.0075f * (distance * distance));

	//Final light
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
	//return (diffuse + specular);
}

void main()
{
	vec3 res1 = calLightPosition0();
	vec3 res2 = calLightPosition1();
	vec3 result = (res1 + res2) + objColor;
	FragColor = vec4(result, 1.0);
}
