#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec3 objColor;
in vec3 ligPos;
in vec3 ligColor;

void main()
{
	//3.2 matrial exer
	// ambient
	float ambientStrength = 0.5;
	vec3 ambient = ambientStrength * ligColor;

	// diffuse
	float diffuseStrength = 0.9;
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(FragPos - ligPos);
	//float diff = clamp(dot(norm, lightDir), 0.0, 1.0);
	float diff = max(dot(norm, lightDir), 1.0);
	//vec3 diffuse = diff * diffuseStrength * ligColor;
	vec3 diffuse = ligColor * (diff * vec3(0.0f, 0.50980392f, 0.50980392f));

	// Specular
	float specularStrength = 0.9;
	vec3 E = normalize(vec3(10.0f, 10.0f, -50.0f));
	vec3 viewDir = normalize(E - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 10.0f);
	vec3 specular = ligColor * (spec * vec3(1.0f, 1.0f, 1.0f));
	//vec3 specular = specularStrength * spec * ligColor;

	//Attenuation
	float distance = length(FragPos - ligPos);
	//constant linear quadratic
	float attenuation = 1.f / (1.f + 0.045f * distance + 0.0075f * (distance * distance));

	//Final light
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;


	//vec3 result = (ambient + diffuse + specular) * objColor ;
	vec3 result = (ambient + diffuse + specular) * objColor ;
	FragColor = vec4(result, 1.0);
}
