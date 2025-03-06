//----------------------------------------------------------------------------------------
/**
 * \file	render.cpp
 * \author	Yahor Byaliauski
 * \date	21.04.2024
 * \brief	Implementation of creating and drawing objects in the scene, initializing shaders,
			global variables, deleteing objects from the scene.
*/
//----------------------------------------------------------------------------------------

#pragma once


#include "render.h"
#include "spline.h"
#include "rock.h"
#include <iostream>

ShaderProgram commonShaderProgram;
SkyboxShaderProgram commonSkyBoxShaderProgram;
ParticleShaderProgram commonParticleShaderProgram;
ParticleShaderProgram commonHoloShaderProgram;
gameState game_state;
std::vector<Wrap> models;

void Camera::updateCameraView(float camv_ang) {
	dir.x = cos(glm::radians(camv_ang));
	dir.y = sin(glm::radians(camv_ang));
}


void cleanupShaderPrograms() {
	pgr::deleteProgramAndShaders(commonShaderProgram.program);
	pgr::deleteProgramAndShaders(commonSkyBoxShaderProgram.program);
	pgr::deleteProgramAndShaders(commonParticleShaderProgram.program);
	pgr::deleteProgramAndShaders(commonHoloShaderProgram.program);
}

void cleanupModels() {
	for (auto& it : models) {
		for (auto& itMesh : it.mesh) {
			cleanupGeometry(itMesh);
		}
	}
}

/**
	* @brief Initialization of material uniforms for shader
	* @param[in] ambient	Ambient element of material
	* @param[in] diffuse	Diffuse element of material
	* @param[in] specular	Spcular element of material
	* @param[in] shininess  Shininess parameter of material
	* @param[in] texture	Texture of material (if used)
*/
void initMatUniforms(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess, GLuint texture) {
	glUniform3fv(commonShaderProgram.locations.materialAmbient, 1, glm::value_ptr(ambient));
	glUniform3fv(commonShaderProgram.locations.materialDiffuse, 1, glm::value_ptr(diffuse));
	glUniform3fv(commonShaderProgram.locations.materialSpecular, 1, glm::value_ptr(specular));
	glUniform1f(commonShaderProgram.locations.materialShininess, shininess);

	if (texture != 0) {
		glUniform1i(commonShaderProgram.locations.useTexture, 1);  
		glUniform1i(commonShaderProgram.locations.texSampler, 0);  
		glActiveTexture(GL_TEXTURE0 + 0);                  
		glBindTexture(GL_TEXTURE_2D, texture);
	}
	else {
		glUniform1i(commonShaderProgram.locations.useTexture, 0);
	}

}
/**
	* @brief Initialization of light uniforms for shader
*/
void initLightUniforms() {
	glUniform3fv(commonShaderProgram.locations.moonAmbient, 1, glm::value_ptr(game_state.moonAmbient));
	glUniform3fv(commonShaderProgram.locations.moonDiffuse, 1, glm::value_ptr(game_state.moonDiffuse));
	glUniform3fv(commonShaderProgram.locations.moonSpecular, 1, glm::value_ptr(game_state.moonSpecular));
	glUniform3fv(commonShaderProgram.locations.moonPosition, 1, glm::value_ptr(game_state.moonPosition));

	glUniform3fv(commonShaderProgram.locations.pointAmbient, 1, glm::value_ptr(game_state.pointAmbient));
	glUniform3fv(commonShaderProgram.locations.pointDiffuse, 1, glm::value_ptr(game_state.pointDiffuse));
	glUniform3fv(commonShaderProgram.locations.pointSpecular, 1, glm::value_ptr(game_state.pointSpecular));
	glUniform3fv(commonShaderProgram.locations.pointPosition, 1, glm::value_ptr(game_state.pointPosition));

	glUniform3fv(commonShaderProgram.locations.flashlightAmbient, 1, glm::value_ptr(game_state.flashlightAmbient));
	glUniform3fv(commonShaderProgram.locations.flashlightDiffuse, 1, glm::value_ptr(game_state.flashlightDiffuse));
	glUniform3fv(commonShaderProgram.locations.flashlightSpecular, 1, glm::value_ptr(game_state.flashlightSpecular));
	glUniform3fv(commonShaderProgram.locations.flashlightPosition, 1, glm::value_ptr(game_state.flashlightPosition));
	glUniform3fv(commonShaderProgram.locations.flashlightDirection, 1, glm::value_ptr(game_state.flashlightDirection));
	glUniform1i(commonShaderProgram.locations.isFlashlightActive, game_state.isFlashlightActive);

	glUniform1i(commonShaderProgram.locations.isFogActive, game_state.isFogActive);
	glUniform1f(commonShaderProgram.locations.fogAlpha, game_state.fogAlpha);
}

void drawSkybox(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
	glUseProgram(commonSkyBoxShaderProgram.program);
	glm::mat4 viewRotation = viewMatrix;
	viewRotation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	glm::mat4 PVmatrix = glm::inverse(projectionMatrix * viewRotation);
	glUniformMatrix4fv(commonSkyBoxShaderProgram.locations.PVmatrix, 1, GL_FALSE, glm::value_ptr(PVmatrix));
	glUniform1i(commonSkyBoxShaderProgram.locations.cubeSampler, 0);
	glUniform1i(commonSkyBoxShaderProgram.locations.isFogActive, game_state.isFogActive);

	glBindVertexArray(models.at(1).mesh.at(0)->vertexArrayObject);
	glBindTexture(GL_TEXTURE_CUBE_MAP, models.at(1).mesh.at(0)->texture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, models.at(1).mesh.at(0)->numTriangles + 2);
	glBindVertexArray(0);
	glUseProgram(0);
}


void drawButterfly(Object* obj, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
	glUseProgram(commonShaderProgram.program);
	std::vector<ObjectGeometry*> geom = models[obj->meshIndex].mesh;
	int count = 0;
	for (auto& i : geom) {
		initMatUniforms(
			i->ambient,
			i->diffuse,
			i->specular,
			i->shininess,
			i->texture
		);
		initLightUniforms();
		glm::mat4 modelMatrix;
		modelMatrix = allignObject(obj->position, obj->direction, glm::vec3(0.0f, 0.0f, 1.0f));
		if (count == 0) {
			modelMatrix = glm::rotate(modelMatrix, 1.0f * sin(1000 * glm::radians(game_state.time)), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else if (count == 3) {
			modelMatrix = glm::rotate(modelMatrix, 1.0f * -sin(1000 * glm::radians(game_state.time)), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		
		modelMatrix = glm::scale(modelMatrix, glm::vec3(obj->size));
		glm::mat4 PVMMatrix = projectionMatrix * viewMatrix * modelMatrix;
		glUniformMatrix4fv(commonShaderProgram.locations.PVMmatrix, 1, GL_FALSE, glm::value_ptr(PVMMatrix));
		glUniformMatrix4fv(commonShaderProgram.locations.viewM, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(commonShaderProgram.locations.modelM, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		const glm::mat4 modelRotationMatrix = glm::mat4(
			modelMatrix[0],
			modelMatrix[1],
			modelMatrix[2],
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelRotationMatrix));
		glUniformMatrix4fv(commonShaderProgram.locations.normM, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		glBindVertexArray(i->vertexArrayObject);
		glDrawElements(GL_TRIANGLES, i->numTriangles * 3, GL_UNSIGNED_INT, 0);
		count++;
	}
	CHECK_GL_ERROR();
	glBindVertexArray(0);
	glUseProgram(0);
	return;
}

void drawObject(Object* obj, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
	glUseProgram(commonShaderProgram.program);
	if (obj->meshIndex == 13 && game_state.holoActive) {
		glUniform1i(commonShaderProgram.locations.galaxy, 1);
	}
	glm::mat4 modelMatrix;
	modelMatrix = glm::translate(glm::mat4(1.0f), obj->position);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(obj->angle), obj->direction);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(obj->size));

	glm::mat4 PVMMatrix = projectionMatrix * viewMatrix * modelMatrix;
	glUniformMatrix4fv(commonShaderProgram.locations.PVMmatrix, 1, GL_FALSE, glm::value_ptr(PVMMatrix));
	glUniformMatrix4fv(commonShaderProgram.locations.viewM, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(commonShaderProgram.locations.modelM, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	const glm::mat4 modelRotationMatrix = glm::mat4(
		modelMatrix[0],
		modelMatrix[1],
		modelMatrix[2],
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelRotationMatrix));
	glUniformMatrix4fv(commonShaderProgram.locations.normM, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	std::vector<ObjectGeometry*> geom = models[obj->meshIndex].mesh;

	for (auto& i : geom) {
		initMatUniforms(
			i->ambient,
			i->diffuse,
			i->specular,
			i->shininess,
			i->texture
		);
		initLightUniforms();
		glBindVertexArray(i->vertexArrayObject);
		glDrawElements(GL_TRIANGLES, i->numTriangles * 3, GL_UNSIGNED_INT, 0);
	}
	CHECK_GL_ERROR();
	if (obj->meshIndex == 13) {
		glUniform1i(commonShaderProgram.locations.galaxy, 0);
	}
	glBindVertexArray(0);
	glUseProgram(0);

	return;
}

void drawRock(Object* obj, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
	glUseProgram(commonShaderProgram.program);
	glm::mat4 modelMatrix;
	modelMatrix = glm::translate(glm::mat4(1.0f), obj->position);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(obj->angle), obj->direction);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(obj->size));

	glm::mat4 PVMMatrix = projectionMatrix * viewMatrix * modelMatrix;
	glUniformMatrix4fv(commonShaderProgram.locations.PVMmatrix, 1, GL_FALSE, glm::value_ptr(PVMMatrix));
	glUniformMatrix4fv(commonShaderProgram.locations.viewM, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(commonShaderProgram.locations.modelM, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	const glm::mat4 modelRotationMatrix = glm::mat4(
		modelMatrix[0],
		modelMatrix[1],
		modelMatrix[2],
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelRotationMatrix));
	glUniformMatrix4fv(commonShaderProgram.locations.normM, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	models.at(10).mesh.at(0)->ambient = glm::vec3(0.5f, 0.5f, 0.5f);
	models.at(10).mesh.at(0)->diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	models.at(10).mesh.at(0)->specular = glm::vec3(0.5f, 0.5f, 0.5f);

	initMatUniforms(
		models.at(10).mesh.at(0)->ambient,
		models.at(10).mesh.at(0)->diffuse,
		models.at(10).mesh.at(0)->specular,
		models.at(10).mesh.at(0)->shininess,
		models.at(10).mesh.at(0)->texture
	);
	initLightUniforms();

	glBindVertexArray(models.at(10).mesh.at(0)->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, 3 * models.at(10).mesh.at(0)->numTriangles, GL_UNSIGNED_INT, 0);
	CHECK_GL_ERROR();
	glBindVertexArray(0);
	glUseProgram(0);
}

void drawGalaxy(Object* obj, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glUseProgram(commonHoloShaderProgram.program);


	glm::mat4 modelMatrix;
	modelMatrix = glm::translate(glm::mat4(1.0f), obj->position);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(obj->angle), obj->direction);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(obj->size));
	glm::mat4 PVMMatrix = projectionMatrix * viewMatrix * modelMatrix;

	glUniformMatrix4fv(commonHoloShaderProgram.locations.viewM, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(commonHoloShaderProgram.locations.PVMmatrix, 1, GL_FALSE, glm::value_ptr(PVMMatrix));
	glUniform1f(commonHoloShaderProgram.locations.time, game_state.time);
	glUniform1i(commonHoloShaderProgram.locations.texSampler, 0);

	CHECK_GL_ERROR();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindVertexArray(models.at(14).mesh.at(0)->vertexArrayObject);
	glBindTexture(GL_TEXTURE_2D, models.at(14).mesh.at(0)->texture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, models.at(14).mesh.at(0)->numTriangles);

	glBindVertexArray(0);
	glUseProgram(0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glDisable(GL_BLEND);
}

void drawParticles(Object* obj, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glUseProgram(commonParticleShaderProgram.program);

	glm::mat4 billboardRotationMatrix = glm::mat4(
		viewMatrix[0],
		viewMatrix[1],
		viewMatrix[2],
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	billboardRotationMatrix = glm::transpose(billboardRotationMatrix);

	glm::mat4 matrix = glm::mat4(1.0f);
	matrix = glm::translate(matrix, obj->position);
	matrix = glm::scale(matrix, glm::vec3(obj->size));
	matrix = matrix * billboardRotationMatrix; 
	glm::mat4 PVMmatrix = projectionMatrix * viewMatrix * matrix;

	glUniformMatrix4fv(commonParticleShaderProgram.locations.viewM, 1, GL_FALSE, glm::value_ptr(viewMatrix));   
	glUniformMatrix4fv(commonParticleShaderProgram.locations.PVMmatrix, 1, GL_FALSE, glm::value_ptr(PVMmatrix));  
	glUniform1f(commonParticleShaderProgram.locations.time, obj->currentTime - obj->startTime);
	glUniform1i(commonParticleShaderProgram.locations.texSampler, 0);

	glBindVertexArray(models.at(11).mesh.at(0)->vertexArrayObject);
	glBindTexture(GL_TEXTURE_2D, models.at(11).mesh.at(0)->texture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, models.at(11).mesh.at(0)->numTriangles);

	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_BLEND);
}


void loadShaderPrograms()
{

	std::vector<GLuint> shaders;
	shaders.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "lightperpixel.vert"));
	shaders.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "lightperpixel.frag"));
	commonShaderProgram.program = pgr::createProgram(shaders);
	commonShaderProgram.locations.position = glGetAttribLocation(commonShaderProgram.program, "position");
	commonShaderProgram.locations.norm = glGetAttribLocation(commonShaderProgram.program, "norm");
	commonShaderProgram.locations.texCoord = glGetAttribLocation(commonShaderProgram.program, "texCoord");

	commonShaderProgram.locations.viewM = glGetUniformLocation(commonShaderProgram.program, "viewM");
	commonShaderProgram.locations.PVMmatrix = glGetUniformLocation(commonShaderProgram.program, "PVMmatrix");
	commonShaderProgram.locations.modelM = glGetUniformLocation(commonShaderProgram.program, "modelM");
	commonShaderProgram.locations.normM = glGetUniformLocation(commonShaderProgram.program, "normM");
	commonShaderProgram.locations.time = glGetUniformLocation(commonShaderProgram.program, "time");

	commonShaderProgram.locations.materialAmbient = glGetUniformLocation(commonShaderProgram.program, "materialAmbient");
	commonShaderProgram.locations.materialDiffuse = glGetUniformLocation(commonShaderProgram.program, "materialDiffuse");
	commonShaderProgram.locations.materialSpecular = glGetUniformLocation(commonShaderProgram.program, "materialSpecular");
	commonShaderProgram.locations.materialShininess = glGetUniformLocation(commonShaderProgram.program, "materialShininess");
	commonShaderProgram.locations.useTexture = glGetUniformLocation(commonShaderProgram.program, "useTexture");

	commonShaderProgram.locations.moonAmbient = glGetUniformLocation(commonShaderProgram.program, "moonAmbient");
	commonShaderProgram.locations.moonDiffuse = glGetUniformLocation(commonShaderProgram.program, "moonDiffuse");
	commonShaderProgram.locations.moonSpecular = glGetUniformLocation(commonShaderProgram.program, "moonSpecular");
	commonShaderProgram.locations.moonPosition = glGetUniformLocation(commonShaderProgram.program, "moonPosition");

	commonShaderProgram.locations.pointAmbient = glGetUniformLocation(commonShaderProgram.program, "pointAmbient");
	commonShaderProgram.locations.pointDiffuse = glGetUniformLocation(commonShaderProgram.program, "pointDiffuse");
	commonShaderProgram.locations.pointSpecular = glGetUniformLocation(commonShaderProgram.program, "pointSpecular");
	commonShaderProgram.locations.pointPosition = glGetUniformLocation(commonShaderProgram.program, "pointPosition");

	commonShaderProgram.locations.flashlightAmbient = glGetUniformLocation(commonShaderProgram.program, "flashlightAmbient");
	commonShaderProgram.locations.flashlightDiffuse = glGetUniformLocation(commonShaderProgram.program, "flashlightDiffuse");
	commonShaderProgram.locations.flashlightSpecular = glGetUniformLocation(commonShaderProgram.program, "flashlightSpecular");
	commonShaderProgram.locations.flashlightPosition = glGetUniformLocation(commonShaderProgram.program, "flashlightPosition");
	commonShaderProgram.locations.flashlightDirection = glGetUniformLocation(commonShaderProgram.program, "flashlightDirection");
	commonShaderProgram.locations.isFlashlightActive = glGetUniformLocation(commonShaderProgram.program, "isFlashlightActive");

	commonShaderProgram.locations.galaxy = glGetUniformLocation(commonShaderProgram.program, "galaxy");

	commonShaderProgram.locations.texSampler = glGetUniformLocation(commonShaderProgram.program, "texSampler");

	commonShaderProgram.locations.fogAlpha = glGetUniformLocation(commonShaderProgram.program, "fogAlpha");
	commonShaderProgram.locations.isFogActive = glGetUniformLocation(commonShaderProgram.program, "isFogActive");
	commonShaderProgram.initialized = true;

	shaders.clear();
	shaders.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "skybox.vert"));
	shaders.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "skybox.frag"));

	commonSkyBoxShaderProgram.program = pgr::createProgram(shaders);
	commonSkyBoxShaderProgram.locations.coord = glGetAttribLocation(commonSkyBoxShaderProgram.program, "coord");
	commonSkyBoxShaderProgram.locations.cubeSampler = glGetUniformLocation(commonSkyBoxShaderProgram.program, "cubeSampler");
	commonSkyBoxShaderProgram.locations.PVmatrix = glGetUniformLocation(commonSkyBoxShaderProgram.program, "PVmatrix");
	commonSkyBoxShaderProgram.locations.isFogActive = glGetUniformLocation(commonSkyBoxShaderProgram.program, "isFogActive");

	shaders.clear();
	shaders.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "particle.vert"));
	shaders.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "particle.frag"));

	commonParticleShaderProgram.program = pgr::createProgram(shaders);
	commonParticleShaderProgram.locations.position = glGetAttribLocation(commonParticleShaderProgram.program, "position");
	commonParticleShaderProgram.locations.texCoord = glGetAttribLocation(commonParticleShaderProgram.program, "texCoord");
	commonParticleShaderProgram.locations.PVMmatrix = glGetUniformLocation(commonParticleShaderProgram.program, "PVMmatrix");
	commonParticleShaderProgram.locations.viewM = glGetUniformLocation(commonParticleShaderProgram.program, "viewM");
	commonParticleShaderProgram.locations.time = glGetUniformLocation(commonParticleShaderProgram.program, "time");
	commonParticleShaderProgram.locations.texSampler = glGetUniformLocation(commonParticleShaderProgram.program, "texSampler");

	shaders.clear();
	shaders.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "holo.vert"));
	shaders.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "holo.frag"));

	commonHoloShaderProgram.program = pgr::createProgram(shaders);
	commonHoloShaderProgram.locations.position = glGetAttribLocation(commonHoloShaderProgram.program, "position");
	commonHoloShaderProgram.locations.texCoord = glGetAttribLocation(commonHoloShaderProgram.program, "texCoord");
	commonHoloShaderProgram.locations.PVMmatrix = glGetUniformLocation(commonHoloShaderProgram.program, "PVMmatrix");
	commonHoloShaderProgram.locations.viewM = glGetUniformLocation(commonHoloShaderProgram.program, "viewM");
	commonHoloShaderProgram.locations.time = glGetUniformLocation(commonHoloShaderProgram.program, "time");
	commonHoloShaderProgram.locations.texSampler = glGetUniformLocation(commonHoloShaderProgram.program, "texSampler");

}

/**
	* @brief Constructor for Wrap class
	* @param[in] name	Name of object file
	* @param[in] path	Route to object file
*/
Wrap::Wrap(const std::string name, const std::string path) : name(name), path(path) {}

void loadRock(ShaderProgram& shader, Wrap& wrap) {
	ObjectGeometry* geom = new ObjectGeometry;

	glGenVertexArrays(1, &(geom->vertexArrayObject));
	glBindVertexArray(geom->vertexArrayObject);

	glGenBuffers(1, &(geom->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, geom->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * rock_n_attribs_per_vertex * rock_n_vertices, rock_vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &(geom->elementBufferObject));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->elementBufferObject);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned int) * rock_n_triangles, rock_triangles, GL_STATIC_DRAW);
	glEnableVertexAttribArray(shader.locations.position);
	glVertexAttribPointer(shader.locations.position, 3, GL_FLOAT, GL_FALSE, rock_n_attribs_per_vertex * sizeof(float), 0);

	glEnableVertexAttribArray(shader.locations.norm);
	glVertexAttribPointer(shader.locations.norm, 3, GL_FLOAT, GL_FALSE, rock_n_attribs_per_vertex * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(shader.locations.texCoord);
	glVertexAttribPointer(shader.locations.texCoord, 2, GL_FLOAT, GL_FALSE, rock_n_attribs_per_vertex * sizeof(float), (void*)(6 * sizeof(float)));

	geom->ambient = glm::vec3(1.0f, 0.0f, 1.0f);
	geom->diffuse = glm::vec3(1.0f, 0.0f, 1.0f);
	geom->specular = glm::vec3(1.0f, 0.0f, 1.0f);
	geom->shininess = 10.0f;
	geom->texture = pgr::createTexture(wrap.path);

	glBindVertexArray(0);
	geom->numTriangles = rock_n_triangles;
	wrap.mesh.push_back(geom);
}

bool loadMesh(const std::string& fileName, ShaderProgram& shader, std::vector<ObjectGeometry*>* meshGeometryVec) {
	Assimp::Importer importer;
	importer.SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE, 1);
	const aiScene* scn = importer.ReadFile(fileName.c_str(), 0
		| aiProcess_Triangulate
		| aiProcess_PreTransformVertices
		| aiProcess_GenSmoothNormals
		| aiProcess_JoinIdenticalVertices);
	if (scn == nullptr) {
		return false;
	}
	aiMesh* mesh;
	ObjectGeometry* geom;
	for (auto i = 0; i < scn->mNumMeshes; i++) {
		mesh = scn->mMeshes[i];
		geom = new ObjectGeometry;
		glGenBuffers(1, &(geom->vertexBufferObject));
		glBindBuffer(GL_ARRAY_BUFFER, geom->vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float) * mesh->mNumVertices, 0, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float) * mesh->mNumVertices, mesh->mVertices);
		glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float) * mesh->mNumVertices, 3 * sizeof(float) * mesh->mNumVertices, mesh->mNormals);
		float* texCoord = new float[2 * mesh->mNumVertices];
		float* tempCoord = texCoord;
		aiVector3D vec;

		if (mesh->HasTextureCoords(0)) {
			for (auto k = 0; k < mesh->mNumVertices; k++) {
				vec = (mesh->mTextureCoords[0])[k];
				*tempCoord++ = vec.x;
				*tempCoord++ = vec.y;
			}
		}

		glBufferSubData(GL_ARRAY_BUFFER, 6 * sizeof(float) * mesh->mNumVertices, 2 * sizeof(float) * mesh->mNumVertices, texCoord);
		unsigned int* indices = new unsigned int[3 * mesh->mNumFaces];
		for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
			indices[f * 3 + 0] = mesh->mFaces[f].mIndices[0];
			indices[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
			indices[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
		}

		glGenBuffers(1, &(geom->elementBufferObject));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->elementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned) * mesh->mNumFaces, indices, GL_STATIC_DRAW);
		
		delete[] indices;

		const aiMaterial* material = scn->mMaterials[mesh->mMaterialIndex];
		aiColor4D color;
		aiString name;
		aiReturn retValue = AI_SUCCESS;
		material->Get(AI_MATKEY_NAME, name);

		if ((retValue = aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color)) != AI_SUCCESS) {
			color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);
		}
		geom->diffuse = glm::vec3(color.r, color.g, color.b);
		if ((retValue = aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &color)) != AI_SUCCESS) {
			color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);
		}
		geom->ambient = glm::vec3(color.r, color.g, color.b);
		if ((retValue = aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &color)) != AI_SUCCESS) {
			color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);
		}
		geom->specular = glm::vec3(color.r, color.g, color.b);

		ai_real shine, strength;
		unsigned int max;

		max = 1;
		if((retValue = aiGetMaterialFloatArray(material, AI_MATKEY_SHININESS, &shine, &max)) != AI_SUCCESS) {
			shine = 1.0f;
		}
		max = 1;
		if ((retValue = aiGetMaterialFloatArray(material, AI_MATKEY_SHININESS_STRENGTH, &strength, &max)) != AI_SUCCESS) {
			strength = 1.0f;
		}

		geom->shininess = shine * strength;
		geom->texture = 0;

		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString path;
			aiReturn texFound = material->GetTexture(aiTextureType_DIFFUSE, 0, &path);
			std::string textureName = path.data;
			size_t found = fileName.find_last_of("/\\");
			if (found != std::string::npos) {
				textureName.insert(0, fileName.substr(0, found + 1));
			}
			std::cout << "Loading diffuse texture\n";
			geom->texture = pgr::createTexture(textureName);
		}
		CHECK_GL_ERROR();

		glGenVertexArrays(1, &(geom->vertexArrayObject));
		glBindVertexArray(geom->vertexArrayObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom->elementBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, geom->vertexBufferObject);
		glEnableVertexAttribArray(shader.locations.position);
		glVertexAttribPointer(shader.locations.position, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(shader.locations.norm);
		glVertexAttribPointer(shader.locations.norm, 3, GL_FLOAT, GL_FALSE, 0, (void*)(3 * sizeof(float) * mesh->mNumVertices));

		glEnableVertexAttribArray(shader.locations.texCoord);
		glVertexAttribPointer(shader.locations.texCoord, 2, GL_FLOAT, GL_FALSE, 0, (void*)(6 * sizeof(float) * mesh->mNumVertices));
		CHECK_GL_ERROR();

		glBindVertexArray(0);
		geom->numTriangles = mesh->mNumFaces;
		meshGeometryVec->push_back(geom);
	}
	return true;
}

void loadSkybox(GLuint shader, Wrap& wrap) {
	ObjectGeometry* geom = new ObjectGeometry;
	float screenCoord[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };

	glGenVertexArrays(1, &(geom->vertexArrayObject));
	glBindVertexArray(geom->vertexArrayObject);
	glGenBuffers(1, &(geom->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, geom->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenCoord), screenCoord, GL_STATIC_DRAW);
	glEnableVertexAttribArray(commonSkyBoxShaderProgram.locations.coord);
	glVertexAttribPointer(commonSkyBoxShaderProgram.locations.coord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	CHECK_GL_ERROR();

	geom->numTriangles = 2;

	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &(geom->texture));
	glBindTexture(GL_TEXTURE_CUBE_MAP, geom->texture);
	
	std::string partPath = std::string(wrap.path);

	pgr::loadTexImage2D(partPath + "/px.png", GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	pgr::loadTexImage2D(partPath + "/nx.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	pgr::loadTexImage2D(partPath + "/pz.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	pgr::loadTexImage2D(partPath + "/nz.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	pgr::loadTexImage2D(partPath + "/py.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	pgr::loadTexImage2D(partPath + "/ny.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	wrap.mesh.push_back(geom);
}

void loadParticle(ParticleShaderProgram& shader, Wrap& wrap) {
	ObjectGeometry* geom = new ObjectGeometry;

	geom->texture = pgr::createTexture(wrap.path);

	glGenVertexArrays(1, &(geom->vertexArrayObject));
	glBindVertexArray(geom->vertexArrayObject);

	glGenBuffers(1, &(geom->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, geom->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * particleNumVertices * 5, particleVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(shader.locations.position);
	glVertexAttribPointer(shader.locations.position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

	glEnableVertexAttribArray(shader.locations.texCoord);
	glVertexAttribPointer(shader.locations.texCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
	geom->numTriangles = particleNumVertices;
	wrap.mesh.push_back(geom);

}

void cleanupGeometry(ObjectGeometry* geom) {
	glDeleteVertexArrays(1, &(geom->vertexArrayObject));
	glDeleteBuffers(1, &(geom->elementBufferObject));
	glDeleteBuffers(1, &(geom->vertexBufferObject));
	if (geom->texture != 0) {
		glDeleteTextures(1, &(geom->texture));
	}
}

void initModels() {
	models.push_back(Wrap("tree1", "data/Tree2/Tree2.obj"));
	models.push_back(Wrap("skybox", "data/Standard-Cube-Map"));
	models.push_back(Wrap("ground", "data/ground/terrain.obj"));
	models.push_back(Wrap("telescope", "data/telescope/telescope.obj"));
	models.push_back(Wrap("butterfly", "data/butterfly/butterfly.obj"));
	models.push_back(Wrap("tent", "data/tent/tent.obj"));
	models.push_back(Wrap("flower1", "data/flower/flower.obj"));
	models.push_back(Wrap("flower2", "data/flower/flower.obj"));
	models.push_back(Wrap("flower3", "data/flower/flower.obj"));
	models.push_back(Wrap("tree2", "data/Tree2/Tree2.obj"));
	models.push_back(Wrap("rock", "data/rock/rock.jpg"));
	models.push_back(Wrap("particles", "data/magic_particles/particles.jpg"));
	models.push_back(Wrap("flashlight", "data/flashlight/flashlight.obj"));
	models.push_back(Wrap("hologramprojector", "data/hologramprojector/hologramprojector.obj"));
	models.push_back(Wrap("galaxy", "data/galaxy/galaxy.jpg"));
	models.push_back(Wrap("bush", "data/bush/bush.obj"));
	for (auto& it : models) {
		if (it.name == "skybox") {
			loadSkybox(commonSkyBoxShaderProgram.program, it);
		}
		else if (it.name == "rock") {
			loadRock(commonShaderProgram, it);
		}
		else if (it.name == "particles") {
			loadParticle(commonParticleShaderProgram, it);
		}
		else if (it.name == "galaxy") {
			loadParticle(commonHoloShaderProgram, it);
			CHECK_GL_ERROR();
		}
		else {
			loadMesh(it.path, commonShaderProgram, &it.mesh);
		}
	}
}