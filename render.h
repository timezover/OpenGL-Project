//----------------------------------------------------------------------------------------
/**
 * \file	render.h
 * \author	Yahor Byaliauski
 * \date	21.04.2024
 * \brief	Implementation of creating and drawing objects in the scene, initializing shaders,
			global variables, deleteing objects from the scene.
*/
//----------------------------------------------------------------------------------------


#pragma once
#ifndef __RENDER_H
#define __RENDER_H


#include "pgr.h"
#include "string"
#define CAMERA_ELEVATION_ANGLE 45.0f
#define SCENE_WIDTH 2.5f
#define SCENE_HEIGHT 2.5f
#define SCENE_DEPTH 1.0f

typedef struct _ObjectGeometry {
	GLuint        vertexBufferObject;   // identifier for the vertex buffer object
	GLuint        elementBufferObject;  // identifier for the element buffer object
	GLuint        vertexArrayObject;    // identifier for the vertex array object
	unsigned int  numTriangles;			// number of triangles in the mesh
	glm::vec3     ambient;
	glm::vec3     diffuse;
	glm::vec3     specular;
	float         shininess;
	GLuint        texture;
} ObjectGeometry;

typedef struct _Object {
	glm::vec3 position;
	glm::vec3 direction;
	float     speed;
	float     size;
	float startTime;
	float currentTime;
	float angle;
	int meshIndex;
} Object;

/**
	* @brief Main shader program of the application.
*/
typedef struct _ShaderProgram {
	GLuint program;
	bool initialized;
	struct {
		GLint position;
		GLint color;
		GLint PVMmatrix;
		GLint viewM;
		GLint normM;
		GLint modelM;
		GLint norm;
		GLint texCoord;
		GLint time;
		GLint moonAmbient;
		GLint moonDiffuse;
		GLint moonSpecular;
		GLint moonPosition;
		GLint materialAmbient;
		GLint materialDiffuse;
		GLint materialSpecular;
		GLint materialShininess;
		GLint useTexture;
		GLint texSampler;
		GLint pointAmbient;
		GLint pointDiffuse;
		GLint pointSpecular;
		GLint pointPosition;
		GLint flashlightAmbient;
		GLint flashlightDiffuse;
		GLint flashlightSpecular;
		GLint flashlightPosition;
		GLint flashlightDirection;
		GLint isFlashlightActive;
		GLint isFogActive;
		GLint fogAlpha;
		GLint galaxy;
	} locations;
	_ShaderProgram() : program(0), initialized(false) {
		locations.position = -1;
		locations.PVMmatrix = -1;
		locations.viewM = -1;
		locations.color = -1;
		locations.norm = -1;
		locations.texCoord = -1;
		locations.time = -1;
		locations.normM = -1;
		locations.modelM = -1;
		locations.materialAmbient = -1;
		locations.materialDiffuse = -1;
		locations.materialShininess = -1;
		locations.materialSpecular = -1;
		locations.moonAmbient = -1;
		locations.moonDiffuse = -1;
		locations.moonPosition = -1;
		locations.moonSpecular = -1;
		locations.texSampler = -1;
		locations.useTexture = -1;
		locations.pointAmbient = -1;
		locations.pointDiffuse = -1;
		locations.pointSpecular = -1;
		locations.pointPosition = -1;
		locations.flashlightAmbient = -1;
		locations.flashlightDiffuse = -1;
		locations.flashlightSpecular = -1;
		locations.flashlightPosition = -1;
		locations.flashlightDirection = -1;
		locations.isFlashlightActive = -1;
		locations.isFogActive = -1;
		locations.fogAlpha = -1;
		locations.galaxy = -1;
	}
} ShaderProgram;

/**
	* @brief Shader program for skybox. 
*/
typedef struct _SkyboxShaderProgram {
	GLuint program;
	struct {
		GLint coord;
		GLint PVmatrix;
		GLint cubeSampler;
		GLint isFogActive;
	} locations;
	_SkyboxShaderProgram() : program(0) {
		locations.coord = -1;
		locations.PVmatrix = -1;
		locations.cubeSampler = -1;
		locations.isFogActive = -1;
	}
} SkyboxShaderProgram;
/**
	* @brief Shader program film-texture and animated texture.
*/
typedef struct _ParticleShaderProgram {
	GLuint program;
	struct {
		GLint position;
		GLint texCoord;
		GLint PVMmatrix;
		GLint viewM;
		GLint time;
		GLint texSampler;
	} locations;
	_ParticleShaderProgram() : program(0) {
		locations.position = -1;
		locations.PVMmatrix = -1;
		locations.viewM = -1;
		locations.time = -1;
		locations.texCoord = -1;
		locations.texSampler = -1;
	}
} ParticleShaderProgram;

/**
	* @brief Camera class for 2 static cameras and 1 dynamic. 
*/
class Camera {
public:
	Camera() {
		pos = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	glm::vec3 dir;
	glm::vec3 pos;
	glm::vec3 up_v;

	void updateCameraView(float camv_ang);
};
/**
	* @brief Structure for containment important variables of application.
	* These variables are used across other functions.
*/
struct gameState {
	int width;
	int height;
	float camElevationAngle;
	float time;
	float camViewAngle;
	float speed = 0.015f;
	Camera camera;
	bool keyMap[4]; //W - 0, A - 1, S - 2, D - 3 
	bool camDynamicMode = true;
	glm::vec3 moonAmbient;
	glm::vec3 moonDiffuse;
	glm::vec3 moonSpecular;
	glm::vec3 moonPosition;

	glm::vec3 pointAmbient;
	glm::vec3 pointDiffuse;
	glm::vec3 pointSpecular;
	glm::vec3 pointPosition;

	glm::vec3 flashlightAmbient;
	glm::vec3 flashlightDiffuse;
	glm::vec3 flashlightSpecular;
	glm::vec3 flashlightPosition;
	glm::vec3 flashlightDirection;
	bool isFlashlightActive;

	bool isFogActive;
	float fogAlpha;

	bool holoActive;

	Camera telescope;
	bool telescopeActive;

	Camera staticView;
	bool staticViewActive;
};
/**
	* @brief Wrap class for identifying objects' path and name. 
*/
class Wrap {
public:
	Wrap(const std::string name, const std::string path);
	std::vector<ObjectGeometry*> mesh;
	std::string name;
	std::string path;
};

/**
	* @brief Deleting all OpenGL shader programs.
*/
void cleanupShaderPrograms();

/**
	* @brief Deleting all models.
*/
void cleanupModels();

/**
	* @brief Deleting all members of ObjectGeometry class.
	* @param[in]	Instance of ObjectGeomtry class that will be deleted
*/
void cleanupGeometry(ObjectGeometry* geom);

/**
	* @brief Function for loading and defining attributes and uniforms locations of all shaders. 
*/
void loadShaderPrograms();

/**
	* @brief Loading all objects' geometry. 
*/
void initModels();

/**
	* @brief Loading simple object geometry with 1 or more mesh.
	* @param[in] fileName		  Name of file where object is located
	* @param[in] shader			  Shader program that will be used in drawing
	* @param[in] meshGeometryVec  Vector of all meshes of object 
*/
bool loadMesh(const std::string& fileName, ShaderProgram& shader, std::vector<ObjectGeometry*>* meshGeometryVec);

/**
	* @brief Function for loading skybox geometry in the scene.
	* @param[in] shader		Shader program that will be used in drawing
	* @param[in] wrap		Instance of Wrap object class		
*/
void loadSkybox(GLuint shader, Wrap& wrap);

/**
	* @brief Function for loading rock geometry in the scene, that was exported from Blender.
	* @param[in] shader		Shader program that will be used
	* @param[in] wrap		Instance of Wrap object class
*/
void loadRock(ShaderProgram& shader, Wrap& wrap);

/**
	* @brief Function for loading film-texture geometry in the scene.
	* @param[in] shader		Shader program that will be used in drawing
	* @param[in] wrap		Instance of Wrap object class
*/
void loadParticle(ParticleShaderProgram& shader, Wrap& wrap);

/**
	* @brief Function for drawing majority of objects in the scene.
	* @param[in] obj			  Object that will be drawn
	* @param[in] viewMatrix		  View matrix for drawing
	* @param[in] projectionMatrix Projection matrix for drawing
*/
void drawObject(Object* obj, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

/**
	* @brief Function for drawing skybox in the scene.
	* @param[in] viewMatrix		  View matrix for drawing
	* @param[in] projectionMatrix Projection matrix for drawing
*/
void drawSkybox(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

/**
	* @brief Function for drawing exported object in the scene.
	* @param[in] obj			  Object that will be drawn
	* @param[in] viewMatrix		  View matrix for drawing
	* @param[in] projectionMatrix Projection matrix for drawing
*/
void drawRock(Object* obj, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

/**
	* @brief Function for drawing film-texure in the scene.
	* @param[in] obj			  Object that will be drawn
	* @param[in] viewMatrix		  View matrix for drawing
	* @param[in] projectionMatrix Projection matrix for drawing
*/
void drawParticles(Object* obj, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

/**
	* @brief Function for drawing animated texture in the scene.
	* @param[in] obj			  Object that will be drawn
	* @param[in] viewMatrix		  View matrix for drawing
	* @param[in] projectionMatrix Projection matrix for drawing
*/
void drawGalaxy(Object* obj, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

/**
	* @brief Function for drawing dynamic object in the scene.
	* @param[in] obj			  Object that will be drawn
	* @param[in] viewMatrix		  View matrix for drawing
	* @param[in] projectionMatrix Projection matrix for drawing
*/
void drawButterfly(Object* obj, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);


#endif