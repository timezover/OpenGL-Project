//----------------------------------------------------------------------------------------
/**
 * \file	particle.vert
 * \author	Yahor Byaliauski
 * \date	26.04.2024
 * \brief	Vertex shader for drawing film-texture in the scene.
*/
//----------------------------------------------------------------------------------------

#version 140

in vec3 position;
in vec2 texCoord;

uniform mat4 PVMmatrix;

smooth out vec2 vertexTexCoord;

void main(){
	gl_Position = PVMmatrix * vec4(position, 1);
	vertexTexCoord = texCoord;
}