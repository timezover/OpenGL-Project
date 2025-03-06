//----------------------------------------------------------------------------------------
/**
 * \file	lightperpixel.vert
 * \author	Yahor Byaliauski
 * \date	22.04.2024
 * \brief	Vertex shader for drawing majority of objects in the scene with Phong lightning model
 *			per pixel.
*/
//----------------------------------------------------------------------------------------

#version 140

in vec3 position;
in vec3 norm;
in vec2 texCoord;

uniform mat4 PVMmatrix; // Projection * View * Model
uniform mat4 viewM; // View matrix
uniform mat4 modelM; // Model matrix
uniform mat4 normM; // Inversed transposed model matrix 

smooth out vec3 vertexPosition;
smooth out vec3 vertexNormal;
smooth out vec2 vertexTexCoord;

void main(){
	vertexPosition = (viewM * modelM * vec4(position, 1.0)).xyz;
	vertexNormal = normalize((viewM * normM * vec4(norm, 0.0)).xyz);
	gl_Position = PVMmatrix * vec4(position, 1);
	vertexTexCoord = texCoord;
}