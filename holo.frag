//----------------------------------------------------------------------------------------
/**
 * \file	holo.frag
 * \author	Yahor Byaliauski
 * \date	10.05.2024
 * \brief	Fragment shader for drawing animated texture in the scene, specifically rotation.
*/
//----------------------------------------------------------------------------------------

#version 140

uniform float time;
uniform mat4 viewM;
uniform sampler2D texSampler;

smooth in vec3 vertexPosition;
smooth in vec2 vertexTexCoord;

out vec4 color_f;

void main(){
	float c = cos(- time / 8); // Speed of rotation
	float s = sin(- time / 8);
	float x = 0.5; // Center of rotation
    float y = 0.5;
    mat3 translation = mat3(1.0, 0.0, 0.0, 
                            0.0, 1.0, 0.0, 
                            -x, -y, 1.0);
    mat3 rotation = mat3( c, s, 0.0, 
                         -s, c, 0.0, 
                         0.0, 0.0, 1.0);
    mat3 translationBack = mat3(1.0, 0.0, 0.0, 
                                0.0, 1.0, 0.0, 
                                x, y, 1.0);
    vec3 a = translationBack * rotation * translation * vec3(vertexTexCoord, 1.0);
    if (a.x < 0.0 || a.x > 1.0 || a.y < 0.0 || a.y > 1.0) { // Deleting texture pieces if texture reaches bounds of itself (avoiding repeating)
        discard; 
    } else {
	    color_f = texture(texSampler, vec2(a.x, a.y));
    }
}