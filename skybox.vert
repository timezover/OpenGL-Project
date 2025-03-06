//----------------------------------------------------------------------------------------
/**
 * \file	skybox.vert
 * \author	Yahor Byaliauski
 * \date	24.04.2024
 * \brief	Vertex shader for drawing skybox with cubemap
*/
//----------------------------------------------------------------------------------------

#version 140

uniform mat4 PVmatrix;

in vec2 coord;

smooth out vec3 texCoord;
smooth out vec3 fogCoord;

void main() {
	vec4 planeCoord = vec4(coord, 0.9999, 1.0);
	vec4 worldCoord = PVmatrix * planeCoord;
	texCoord = worldCoord.xyz / worldCoord.w;
	fogCoord = vec3(planeCoord.x, planeCoord.y, planeCoord.z);
	gl_Position = planeCoord;
}