#version 150

out vec4 fragColor;

uniform mat4 model;
uniform vec3 cameraPosition;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 lightPos1;
uniform float diffuseStrength;
uniform float specularStrength;
uniform sampler2D myTextureSampler;

in vec3 fragVertex;
in vec3 normalVertex;
in vec2 textureCoord;

vec3 calculateLightPosition()
{
	vec3 normal = normalize(transpose(inverse(mat3(model))) * normalVertex);
	vec3 surfacePos = vec3(model * vec4(fragVertex, 1));
	vec3 surfaceToLight = normalize(lightPos - surfacePos);

	// Ambient
	vec3 ambient = 0.05 * objectColor;

	// diffuse
	float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * diffuseStrength * objectColor * lightColor;

	// Specular
	vec3 incidenceVector = -surfaceToLight; //a unit vector
	vec3 reflectionVector = reflect(incidenceVector, normal);
	vec3 surfaceToCamera = normalize(cameraPosition - surfacePos);
	//vec3 surfaceToCamera = normalize(vec3(0) - surfacePos);
	float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
	float specularCoefficient = pow(cosAngle, 40.0);

	if (!(diffuseCoefficient > 0.0))
		specularCoefficient = 0.0;

	vec3 specular = specularCoefficient * specularStrength * lightColor;

	return (ambient + diffuse + specular);
	float distanceToLight = length(lightPos - surfacePos);
	float attenuation = 1.0 / (1.0 + 0.2f * pow(distanceToLight, 2));

	//linear color (color before gamma correction)
	vec3 linearColor = ambient + attenuation*(diffuse + specular);
	//final color (after gamma correction)
	vec3 gamma = vec3(1.0/2.2);
	vec3 result = vec3(pow(linearColor, gamma));
	return result;
}

vec3 calculateLightPosition1()
{
	vec3 normal = normalize(transpose(inverse(mat3(model))) * normalVertex);
	vec3 surfacePos = vec3(model * vec4(fragVertex, 1));
	vec3 surfaceToLight = normalize(lightPos1 - surfacePos);

	// Ambient
	vec3 ambient = 0.05 * objectColor;

	// diffuse
	float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * diffuseStrength * objectColor * lightColor;

	// Specular
	vec3 incidenceVector = -surfaceToLight; //a unit vector
	vec3 reflectionVector = reflect(incidenceVector, normal);
	vec3 surfaceToCamera = normalize(cameraPosition - surfacePos);
	float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
	float specularCoefficient = pow(cosAngle, 40.0);

	if (!(diffuseCoefficient > 0.0))
		specularCoefficient = 0.0;

	vec3 specular = specularCoefficient * specularStrength * lightColor;

	return (ambient + diffuse + specular);
	float distanceToLight = length(lightPos - surfacePos);
	float attenuation = 1.0 / (1.0 + 0.2f * pow(distanceToLight, 2));

	//linear color (color before gamma correction)
	vec3 linearColor = ambient + attenuation*(diffuse + specular);
	//final color (after gamma correction)
	vec3 gamma = vec3(1.0/2.2);
	vec3 result = vec3(pow(linearColor, gamma));
	return result;
}

void main() {
	vec3 result = calculateLightPosition();
	vec3 result1 = calculateLightPosition1();
	//fragColor = vec4(result + result1, 1.0);
	fragColor = vec4(texture(myTextureSampler, textureCoord).rgb, 1.0);
	//fragColor = vec4(result, 1.0);
}
