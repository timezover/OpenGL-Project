//----------------------------------------------------------------------------------------
/**
 * \file	holo.vert
 * \author	Yahor Byaliauski
 * \date	10.05.2024
 * \brief	Vertex shader for drawing animated texture in the scene, specifically rotation.
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