//----------------------------------------------------------------------------------------
/**
 * \file	lightperpixel.frag
 * \author	Yahor Byaliauski
 * \date	22.04.2024
 * \brief	Vertex shader for drawing majority of objects in the scene with Phong lightning model
 *			per pixel.
*/
//----------------------------------------------------------------------------------------

#version 140

smooth in vec3 vertexPosition;
smooth in vec3 vertexNormal;
smooth in vec2 vertexTexCoord;

uniform sampler2D texSampler;
uniform float time;

/** Directional light components: 
	Ambient
	Diffuse
	Specular
	Position
*/
uniform vec3 moonAmbient;
uniform vec3 moonDiffuse;
uniform vec3 moonSpecular;
uniform vec3 moonPosition;

/** Point light components: 
	Ambient
	Diffuse
	Specular
	Position
*/
uniform vec3 pointAmbient;
uniform vec3 pointDiffuse;
uniform vec3 pointSpecular;
uniform vec3 pointPosition;

/** Spotlight components: 
	Ambient
	Diffuse
	Specular
	Position
	Direction
*/
uniform vec3 flashlightAmbient;
uniform vec3 flashlightDiffuse;
uniform vec3 flashlightSpecular;
uniform vec3 flashlightPosition;
uniform vec3 flashlightDirection;

uniform bool isFlashlightActive; // True if projector light is active in the scene, otherwise false

uniform bool isFogActive; // True if fog is active in the scene is active, otherwise false
uniform float fogAlpha;

uniform bool galaxy; // True if hologram projector in the scene is active, otherwise false

/** Material components: 
	Ambient
	Diffuse
	Specular
	Shininess
*/
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

uniform int useTexture; // If object is using texture

out vec4 color_f;

vec4 calculateMoon(){ // Calculating light according to direction light
	float diffuseComp = max(0.0, dot(vertexNormal, normalize(moonPosition)));
	float specularComp = max(0.0, dot(reflect(-normalize(moonPosition), vertexNormal), normalize(-vertexPosition)));

	vec3 res = materialAmbient * moonAmbient;
	res += materialDiffuse * moonDiffuse * diffuseComp;
	res += materialSpecular * moonSpecular * pow(specularComp, materialShininess);

	return vec4(res, 1.0);
}

vec4 calculatePoint(){ // Calculating light according to point light
	float dist = length(pointPosition - vertexPosition);
	float diffuseComp = max(0.0, dot(vertexNormal, normalize(pointPosition - vertexPosition)) / dist);
	float specularComp = max(0.0, dot(reflect(-normalize(pointPosition - vertexPosition), vertexNormal), normalize(-vertexPosition)));

	vec3 res = materialAmbient * pointAmbient;
	res += materialDiffuse * pointDiffuse * diffuseComp;
	res += materialSpecular * pointSpecular * pow(specularComp, materialShininess);
	res /= max(1.0f, (dist * dist) * 2);

	res = mix(res, vec3(0.0), 1.0 - (cos(10.0 * time / 5.0) + 1.5)); // Changing intensity of light according to time parameter
	return vec4(res, 1.0);
}

vec4 calculateSpotlight(){ // Calculating light according to spotlight
	if (!isFlashlightActive){
		return vec4(0.0);
	}
	float diffuseComp = max(0.0, dot(vertexNormal, normalize(flashlightPosition - vertexPosition)));
	float specularComp = max(0.0, dot(reflect(-normalize(flashlightPosition - vertexPosition), vertexNormal), normalize(-vertexPosition)));
	float spotlightFactor = max(0.0, dot(-normalize(flashlightPosition - vertexPosition), flashlightDirection));

	vec3 res = materialAmbient * flashlightAmbient;
	res += materialDiffuse * flashlightDiffuse * diffuseComp;
	res += materialSpecular * flashlightSpecular * pow(specularComp, materialShininess);

	if (spotlightFactor < 0.96){
		res *= 0.0;
	} else {
		res *= pow(spotlightFactor, 1.0);
	}
	return vec4(res, 1.0);
}

void main(){
	vec3 globalAmbient = vec3(0.2);
	vec4 color = vec4(materialAmbient * globalAmbient, 0.0);
	
	color += calculateMoon();
	color += calculatePoint();
	color += calculateSpotlight();
	color_f = color;
	if (useTexture == 1){
		color_f = color * texture(texSampler, vertexTexCoord);
		if (galaxy){
			color_f *= 2.5;
		}
	}
	if (isFogActive){ // Calculating color according to fog parameters
		float alpha = exp(-pow(fogAlpha * length(vertexPosition), 2));
		if (alpha < 0.0){
			alpha = 0.0;
		}
		if (alpha > 1.0){
			alpha = 1.0;
		}
		color_f = mix(vec4(0.3, 0.3, 0.37, 1.0), color_f, alpha);
	}
}