//----------------------------------------------------------------------------------------
/**
 * \file	particle.frag
 * \author	Yahor Byaliauski
 * \date	26.04.2024
 * \brief	Fragment shader for drawing film-texture in the scene.
*/
//----------------------------------------------------------------------------------------

#version 140

uniform float time;
uniform mat4 viewM; // View matrix
uniform sampler2D texSampler;
uniform ivec2 pattern = ivec2(8, 8); // Count of images in a row and column

smooth in vec3 vertexPosition;
smooth in vec2 vertexTexCoord;

out vec4 color_f;

float duration = 0.1f; // Duration of one frame

vec4 sampleTexture(int frame) { // Defining which frame will be drawn according to time
  vec2 offset = vec2(1.0) / vec2(pattern);
  vec2 texCoordBase = vertexTexCoord / vec2(pattern);
  vec2 texCoord = texCoordBase + vec2(frame % pattern.x, pattern.y - 1 - (frame / pattern.x)) * offset;

  return texture(texSampler, texCoord);
}

void main(){
	int frame = int(time / duration);
	color_f = sampleTexture(frame) * vec4(0.6, 0.6, 0.6, 1.0);
}