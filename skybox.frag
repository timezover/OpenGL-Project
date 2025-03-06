//----------------------------------------------------------------------------------------
/**
 * \file	skybox.frag
 * \author	Yahor Byaliauski
 * \date	24.04.2024
 * \brief	Fragment shader for drawing skybox with cubemap
*/
//----------------------------------------------------------------------------------------

#version 140

uniform samplerCube cubeSampler;
uniform bool isFogActive; // True if fog is active, otherwise false

smooth in vec3 texCoord;
smooth in vec3 fogCoord;

out vec4 color_f;

void main() {
	color_f = texture(cubeSampler, texCoord);
	if (isFogActive){
		float alpha = exp(-pow(length(fogCoord), 2));
		if (alpha < 0.0){
			alpha = 1.0;
		}
		if (alpha > 1.0){
			alpha = 1.0;
		}
		color_f = mix(vec4(0.3, 0.3, 0.35, 1.0), color_f, alpha);
	}
}