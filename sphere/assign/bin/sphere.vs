#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 normals;

out vec3 Normal;
out vec3 FragPos;
out vec3 objColor;
out vec3 ligPos;
out vec3 ligColor;

// Values that stay constant for the whole mesh.
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos; 
  
void main(){
	FragPos = vec3(model * vec4(vertexPosition_modelspace, 1.0));
	Normal = mat3(transpose(inverse(model))) * normals;
	//Normal = mat3(model) * normals;
	gl_Position = projection * view * vec4(FragPos, 1.0);
	objColor = vec3(objectColor);
	ligPos = vec3(lightPos);
	ligColor = vec3(lightColor);
}
