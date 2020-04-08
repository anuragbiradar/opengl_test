#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;

//out vec3 Normal;
//out vec3 FragPos;
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
	//FragPos = vec3(model * vec4(vertexPosition_modelspace, 1.0));
	//Normal = mat3(transpose(inverse(model))) * normals;
	UV = uvs;
	//Normal = mat3(model) * normals;
	gl_Position = projection * view * model * vec4(vertexPosition_modelspace, 1.0);
}
