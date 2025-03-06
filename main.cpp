//----------------------------------------------------------------------------------------
/**
 * \file    main.cpp
 * \author  Yahor Byaliauski
 * \date    21.04.2024
 * \brief   Main file of a project, that joins all activity in the scene, such as 
			creating, updating and deleting objects, moving, animation, etc. 
 */

 /**
  * \brief	\mainpage Documentation of the skeleton for the Computer graphics course on FEE and FIT CTU CZ
  *
  * This is the skeleton of an OpenGL application written in FreeGLUT and PGR framework.
  * It serves as a starter boiler-plate code for a semester project - a simple interactive application.
  * Write your code into the functions prepared. Add your data (models, textures, ...) and logic.
  *
  *
  * Comment your code using the [doxygen](https://www.doxygen.nl/index.html) documenting system style.
  * Create "doxygen" directory, make it current by "cd doxygen", prepare a configuration file with "doxygen -g" and edit the details.
  *
  * Start by renaming of this file from skeleton.cpp to <your_name>.cpp and the project to <your_name>.vcxproj
  *
  * In <your_name>.vcxproj:
  *   - rename skeleton in <RootNamespace>skeleton</RootNamespace> to <your_name>
  *   - replace skeleton.cpp in <ClCompile Include="skeleton.cpp" /> to <your_name>.cpp
  *
  * Start coding and documenting. Generate the documentation by the command "doxygen" in the "doxygen" directory.
  *
  */

#pragma once

#include <iostream>
#include "pgr.h"
#include "render.h"
#include "spline.h"

constexpr int WINDOW_WIDTH = 750;
constexpr int WINDOW_HEIGHT = 750;
constexpr char WINDOW_TITLE[] = "Stargazing";

extern ShaderProgram commonShaderProgram;
extern gameState game_state;
std::vector<Object*> objects;


/**
	* @brief Creating an object with index which defines its order of creating and drawing in the scene.
	* @param[in] position     Object position
	* @param[in] rotation     Rotation of object
	* @param[in] angle        Angle for rotation
	* @param[in] size         Object size
	* @param[in] meshIndex    Object index
	* @param[in] speed        Speed of object if dynamic, otherwise 0 for static
 */
Object* initObject(glm::vec3 position, glm::vec3 rotation, float angle, float size, int meshIndex, float speed) {
	Object* object = new Object();
	object->position = position;
	object->angle = angle;
	object->direction = rotation;
	object->speed = speed;
	object->size = size;
	object->startTime = game_state.time;
	object->currentTime = object->startTime;
	object->meshIndex = meshIndex;
	return object;
}

/**
	* @brief Mouse function for calculating camera rotation.
	* @param[in] mouseX       X coordinate of a mouse
	* @param[in] mouseY       Y coordinate of a mouse
 */
void passiveMouseMotionCb(int mouseX, int mouseY) {
	if (mouseY != game_state.height / 2) {
		const float camElevAngle = 0.5f * (game_state.height / 2 - mouseY) / 2;
		if (fabs(game_state.camElevationAngle + camElevAngle) < CAMERA_ELEVATION_ANGLE) {
			game_state.camElevationAngle += camElevAngle;
		}
		glutWarpPointer(game_state.width / 2, game_state.height / 2);
		glutPostRedisplay();
	}

	if (mouseX != game_state.width / 2) {
		const float camViewAngle = 0.5f * (mouseX - game_state.width / 2) / 2;
		game_state.camViewAngle -= camViewAngle;
		if (game_state.camViewAngle < 0.0f) {
			game_state.camViewAngle += 360.0f;
		}
		else if (game_state.camViewAngle > 360.0f) {
			game_state.camViewAngle -= 360.0f;
		}
		game_state.camera.updateCameraView(game_state.camViewAngle);
		glutWarpPointer(game_state.width / 2, game_state.height / 2);
		glutPostRedisplay();
	}
}

/**
	* @brief Deleteing objects before exiting application.
*/
void cleanUpObjects(void) {
	for (auto& it : objects) {
		if (it != NULL) {
			delete it;
			it = NULL;
		}
	}
}

/**
	* @brief Function checks if collision with an object happened.
	* @param[in] position	Position of camera	
*/
bool checkTentCollision(const glm::vec3 &position) {
	float dist = 0.25f;
	float x = objects.at(4)->position.x;
	float y = objects.at(4)->position.y;
	if (distance(glm::vec2(position.x, position.y), glm::vec2(x, y)) > dist) {
		return false;
	}
	return true;
}

/**
	* @brief Function checks if collision with world boundaries happened.
	* @param[in] position	Position of camera
*/
bool checkWorldCollision(const glm::vec3 &position) {
	if (position.x < -SCENE_WIDTH || SCENE_WIDTH < position.x) {
		return true;
	}
	if (position.y < -SCENE_HEIGHT || SCENE_HEIGHT < position.y) {
		return true;
	}
	return false;
}

/**
	* @brief Loop function that is used to draw objects in the scene until application exit.
*/
void drawWindowContents() {
	if (game_state.telescopeActive == true || game_state.staticViewActive == true) {
		glutPassiveMotionFunc(NULL);
		glutSetCursor(GLUT_CURSOR_NONE);
	}
	else {
		glutPassiveMotionFunc(passiveMouseMotionCb);
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}
	glm::vec3 statPos, statDir, statUp;
	if (game_state.telescopeActive) {
		statPos = game_state.telescope.pos;
		statDir = game_state.telescope.dir;
		statUp = game_state.telescope.up_v;
	}
	if (game_state.staticViewActive) {
		statPos = game_state.staticView.pos;
		statDir = game_state.staticView.dir;
		statUp = game_state.staticView.up_v;
	}
	glm::mat4 orthoProjectionMatrix = glm::ortho(
		-SCENE_WIDTH, SCENE_WIDTH,
		-SCENE_HEIGHT, SCENE_HEIGHT,
		-10.0f * SCENE_DEPTH, 10.0f * SCENE_DEPTH
	);
	glm::mat4 orthoViewMatrix = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	glm::mat4 viewMatrix = orthoViewMatrix;
	glm::mat4 projectionMatrix = orthoProjectionMatrix;

	if (game_state.camDynamicMode) {
		glm::vec3 cameraPosition = game_state.camera.pos;
		glm::vec3 cameraUpVector = game_state.camera.up_v;
		glm::vec3 cameraViewDirection = game_state.camera.dir;
		glm::vec3 rotationAxis = glm::cross(cameraViewDirection, cameraUpVector);
		glm::mat4 cameraTransform = glm::rotate(glm::mat4(1.0f), glm::radians(game_state.camElevationAngle), rotationAxis);
		cameraUpVector = glm::vec3(cameraTransform * glm::vec4(cameraUpVector, 0.0f));
		cameraViewDirection = glm::vec3(cameraTransform * glm::vec4(cameraViewDirection, 0.0f));
		viewMatrix = glm::lookAt(
			cameraPosition,
			cameraPosition + cameraViewDirection,
			cameraUpVector
		);
	}
	else {
		viewMatrix = glm::lookAt(
			statPos,
			statPos + statDir,
			statUp
		);
	}
	CHECK_GL_ERROR();
	game_state.pointPosition = glm::vec3(viewMatrix * glm::vec4(objects.at(3)->position.x, objects.at(3)->position.y, objects.at(3)->position.z + 0.01f, 1.0f));
	game_state.flashlightPosition = glm::vec3(viewMatrix * glm::vec4(objects.at(11)->position.x, objects.at(11)->position.y + 0.01f, objects.at(11)->position.z, 1.0f));
	game_state.flashlightDirection = normalize(glm::vec3(-viewMatrix * glm::vec4(objects.at(11)->direction, 0.0f)));
	projectionMatrix = glm::perspective(glm::radians(60.0f), game_state.width / (float)game_state.height, 0.1f, 10.0f);
	glUseProgram(commonShaderProgram.program);
	glUniform1f(commonShaderProgram.locations.time, game_state.time);
	glUseProgram(0);

	drawSkybox(viewMatrix, projectionMatrix);

	for (auto& i : objects) {
		if (i->meshIndex == 4) {
			drawButterfly(i, viewMatrix, projectionMatrix);
			CHECK_GL_ERROR();
		}
		else if (i->meshIndex == 3 || i->meshIndex == 12 || i->meshIndex == 13) {
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			CHECK_GL_ERROR();
			glStencilFunc(GL_ALWAYS, i->meshIndex, -1);
			drawObject(i, viewMatrix, projectionMatrix);
			glDisable(GL_STENCIL_TEST);
		}
		else if (i->meshIndex == 10) {
			drawRock(i, viewMatrix, projectionMatrix);
			CHECK_GL_ERROR();
		}
		else if (i->meshIndex == 11) {
			drawParticles(i, viewMatrix, projectionMatrix);
		}
		else if (i->meshIndex == 14 && game_state.holoActive) {
			drawGalaxy(i, viewMatrix, projectionMatrix);
			CHECK_GL_ERROR();
		}
		else {
			drawObject(i, viewMatrix, projectionMatrix);
			CHECK_GL_ERROR();
		}
		CHECK_GL_ERROR();
	}

}

/**
	* @brief Loop function that is used for drawing objects in the scene
 */
void displayCb() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	drawWindowContents();

	glutSwapBuffers();
}

/**
	* @brief Window was reshaped.
	* @param[in]	newWidth New window width
	* @param[in]	newHeight New window height
 */
void reshapeCb(int newWidth, int newHeight) {
	game_state.width = newWidth;
	game_state.height = newHeight;
	glViewport(0, 0, (GLsizei)newWidth, (GLsizei)newHeight);
};

// Called whenever a key on the keyboard was released. The key is given by
// the "keyReleased" parameter, which is in ASCII. 
/**
	* @brief Handle the key released event.
	* @param[in] keyReleased ASCII code of the released key
	* @param[in] mouseX mouse (cursor) X position
	* @param[in] mouseY mouse (cursor) Y position
 */
void keyboardUpCb(unsigned char keyReleased, int mouseX, int mouseY) {
	switch (keyReleased) {
	case 27:
		glutLeaveMainLoop();
		exit(EXIT_SUCCESS);
	case 'w':
		game_state.keyMap[0] = false;
		break;
	case 'a':
		game_state.keyMap[1] = false;
		break;
	case 's':
		game_state.keyMap[2] = false;
		break;
	case 'd':
		game_state.keyMap[3] = false;
		break;
	}
}

/**
	* @brief Handle the non-ASCII key pressed event (such as arrows or F1).
	*  The special keyboard callback is triggered when keyboard function (Fx) or directional
	*  keys are pressed.
	* @param[in] specKeyPressed int value of a predefined glut constant such as GLUT_KEY_UP
	* @param[in] mouseX mouse (cursor) X position
	* @param[in] mouseY mouse (cursor) Y position
 */
void specialKeyboardCb(int specKeyPressed, int mouseX, int mouseY) {
	switch (specKeyPressed) {
	case GLUT_KEY_F1:
		if (game_state.camDynamicMode) {
			game_state.camDynamicMode = false;
			game_state.staticViewActive = true;
		}
		else {
			if (game_state.telescopeActive) {
				break;
			}
			glutWarpPointer(game_state.width / 2, game_state.height / 2);
			game_state.camDynamicMode = true;
			game_state.staticViewActive = false;
		}
	}
}
 
/**
	* @brief React to mouse button press and release (mouse click).
	* When the user presses and releases mouse buttons in the window, each press
	* and each release generates a mouse callback (including release after dragging).
	*
	* @param[in] buttonPressed button code (GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, or GLUT_RIGHT_BUTTON)
	* @param[in] buttonState GLUT_DOWN when pressed, GLUT_UP when released
	* @param[in] mouseX mouse (cursor) X position
	* @param[in] mouseY mouse (cursor) Y position
 */
void mouseCb(int buttonPressed, int buttonState, int mouseX, int mouseY) {
	if (buttonPressed == GLUT_RIGHT_BUTTON && buttonState == GLUT_DOWN && game_state.telescopeActive == true) {
		game_state.camDynamicMode = true;
		game_state.telescopeActive = false;
		glutWarpPointer(game_state.width / 2, game_state.height / 2);
		return;
	}
	if (buttonPressed != GLUT_LEFT_BUTTON || buttonState != GLUT_DOWN || !game_state.camDynamicMode) {
		return;
	}
	unsigned char id = 0;
	glReadPixels(mouseX, game_state.height - mouseY, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &id);
	/*std::cout << "Position:" << game_state.camera.pos.x << " " << game_state.camera.pos.y << " " << game_state.camera.pos.z << "\n";
	std::cout << "Direction:" << game_state.camera.dir.x << " " << game_state.camera.dir.y << " " << game_state.camera.dir.z << "\n";
	std::cout << "Upvector:" << game_state.camera.up_v.x << " " << game_state.camera.up_v.y << " " << game_state.camera.up_v.z << "\n";*/
	if (id == 3) {
		game_state.camDynamicMode = false;
		game_state.telescopeActive = true;
	}
	if (id == 12) {
		game_state.isFlashlightActive = !game_state.isFlashlightActive;
	}
	if (id == 13) {
		game_state.holoActive = !game_state.holoActive;
	}
}

/**
	* @brief Updating dynamic objects in the scene.
*/
void updateObjects(float elapsedTime) {
	if (objects.at(3) != NULL) {
		objects.at(3)->currentTime = elapsedTime;
		float time_ = elapsedTime - objects.at(3)->startTime;
		float speed_ = time_ * objects.at(3)->speed;
		objects.at(3)->direction = glm::normalize(evaluateClosedCurve_1stDerivative(curveButterflyData, curveButterflySize, speed_));
		objects.at(3)->position = evaluateClosedCurve(curveButterflyData, curveButterflySize, speed_) + glm::vec3(0.35f, 0.44f, -0.1f);
	}
	if (objects.at(10) != NULL) {
		objects.at(10)->currentTime = elapsedTime;
	}
}

/**
 * \brief Callback responsible for the scene update.
 */
void timerCb(int)
{
	float elapsedTime = 0.001f * static_cast<float>(glutGet(GLUT_ELAPSED_TIME)); // milliseconds => seconds
	game_state.time = elapsedTime;
	if (game_state.keyMap[0] == true) {
		glm::vec3 newPos = game_state.camera.pos + game_state.camera.dir * game_state.speed;
		if (!checkTentCollision(newPos) && !checkWorldCollision(newPos)) {
			game_state.camera.pos = newPos;
		}
	}
	if (game_state.keyMap[2] == true) {
		glm::vec3 newPos = game_state.camera.pos - game_state.camera.dir * game_state.speed;
		if (!checkTentCollision(newPos) && !checkWorldCollision(newPos)) {
			game_state.camera.pos = newPos;
		}
	}
	if (game_state.keyMap[1] == true) {
		glm::vec3 newPos = game_state.camera.pos + glm::vec3(-game_state.camera.dir.y, game_state.camera.dir.x, game_state.camera.dir.z) * game_state.speed;
		if (!checkTentCollision(newPos) && !checkWorldCollision(newPos)) {
			game_state.camera.pos = game_state.camera.pos + glm::vec3(-game_state.camera.dir.y, game_state.camera.dir.x, game_state.camera.dir.z) * game_state.speed;
		}
	}
	if (game_state.keyMap[3] == true) {
		glm::vec3 newPos = game_state.camera.pos - glm::vec3(-game_state.camera.dir.y, game_state.camera.dir.x, game_state.camera.dir.z) * game_state.speed;
		if (!checkTentCollision(newPos) && !checkWorldCollision(newPos)){
			game_state.camera.pos = newPos;
		}
	}
	updateObjects(elapsedTime);
	glutTimerFunc(33, timerCb, 0);

	glutPostRedisplay();
}

/**
	* @brief Deleteing objects before application exit. 
*/
void delObjects() {
	for (auto& i : objects) {
		if (i != nullptr) {
			delete i;
			i = nullptr;
		}
	}
}
/**
	* @brief Initialization of camera, lightning and other data in the scene.
	* Restart function is called at the beginning of application or if user presses special key to reset an application.
*/
void restart() {
	delObjects();
	game_state.time = 0.001f * (float)glutGet(GLUT_ELAPSED_TIME);

	game_state.camera.pos = glm::vec3(-1.85f, 1.0f, 0.0f);
	game_state.camera.dir = glm::vec3(1.0f, 0.0f, 0.0f);
	game_state.camera.up_v = glm::vec3(0.0f, 0.0f, 1.0f);
	game_state.camDynamicMode = true;
	game_state.camElevationAngle = 0.0f;
	game_state.camViewAngle = 0.0f;
	game_state.telescopeActive = false;

	game_state.telescope.pos = glm::vec3(-3.5f, -3.0f, 4.5f);
	game_state.telescope.dir = glm::vec3(-1.0f, -1.0f, 0.8f);
	game_state.telescope.up_v = glm::vec3(0.0f, 0.0f, 1.0f);

	game_state.moonAmbient = glm::vec3(0.0f, 0.0f, 0.1f);
	game_state.moonDiffuse = glm::vec3(0.3f, 0.3f, 0.5f);
	game_state.moonSpecular = glm::vec3(0.2f, 0.2f, 0.2f);
	game_state.moonPosition = glm::vec3(1.0f, 1.0f, 1.0f);

	game_state.pointAmbient = glm::vec3(0.0f, 0.0f, 0.0f);
	game_state.pointDiffuse = glm::vec3(0.2f, 0.2f, 0.9f);
	game_state.pointSpecular = glm::vec3(0.1f, 0.1f, 0.8f);

	game_state.flashlightAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
	game_state.flashlightDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	game_state.flashlightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
	game_state.isFlashlightActive = false;

	game_state.isFogActive = false;
	game_state.fogAlpha = 0.5f;

	game_state.holoActive = false;

	game_state.staticViewActive = false;
	game_state.staticView.pos = glm::vec3(-1.82177f, - 1.83475f, 0.0f);
	game_state.staticView.dir = glm::vec3(0.573576f, 0.819152f, 0.0f);
	game_state.staticView.up_v = glm::vec3(0.0f, 0.0f, 1.0f);

	glutPassiveMotionFunc(passiveMouseMotionCb);
	glutWarpPointer(game_state.width / 2, game_state.height / 2);

	for (auto i = 0; i < 4; i++) {
		game_state.keyMap[i] = false;
	}
	objects.clear();
	objects.push_back(initObject(glm::vec3(-1.23f, 1.39f, 0.1f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, 0.7f, 0, 0.0f)); // tree
	objects.push_back(initObject(glm::vec3(0.0f, 0.0f, -0.4f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, 3.0f, 2, 0.0f)); // ground
	objects.push_back(initObject(glm::vec3(-1.2f, 0.5f, -0.2f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, 0.2f, 3, 0.0f)); //telescope
	objects.push_back(initObject(glm::vec3(1.5f, 1.6f, 0.1f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, 0.05f, 4, 0.4f)); //butterfly
	objects.push_back(initObject(glm::vec3(-1.4f, 0.0f, -0.19f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, 0.2f, 5, 0.0f)); //tent
	objects.push_back(initObject(glm::vec3(2.0f, 0.8f, -0.24f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, 0.04f, 6, 0.0f)); // flower1
	objects.push_back(initObject(glm::vec3(1.0f, 0.8f, -0.3f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, 0.08f, 7, 0.0f)); //flower2
	objects.push_back(initObject(glm::vec3(0.72f, 0.07f, -0.22f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, 0.08f, 8, 0.0f)); //flower3
	objects.push_back(initObject(glm::vec3(1.23f, 1.1f, 0.15f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, 0.7f, 9, 0.0f)); //tree2
	objects.push_back(initObject(glm::vec3(-0.5f, -1.5f, -0.2f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, 0.2f, 10, 0.0f)); //rock
	objects.push_back(initObject(glm::vec3(0.72f, 0.07f, -0.14f), glm::vec3(1.0f, 0.0f, 0.0f), 180.0f, 0.05f, 11, 0.0f)); //magic
	objects.push_back(initObject(glm::vec3(-1.32f, 0.14f, -0.3f), glm::vec3(0.0f, -1.0f, 0.0f), 0.0f, 0.05f, 12, 0.0f)); //flashlight
	objects.push_back(initObject(glm::vec3(-1.6f, -1.5f, -0.33f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, 0.2f, 13, 0.0f)); //holo
	objects.push_back(initObject(glm::vec3(-1.6f, -1.5f, -0.19f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, 0.2f, 14, 0.0f)); //galaxy
	objects.push_back(initObject(glm::vec3(-0.163792f, -0.844384f, -0.15f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, 0.15f, 15, 0.0f)); //bush
}

/**
 * @brief Handle the key pressed event.
 * Called whenever a key on the keyboard was pressed. The key is given by the "keyPressed"
 * parameter, which is an ASCII character. It's often a good idea to have the escape key (ASCII value 27)
 * to call glutLeaveMainLoop() to exit the program.
 * @param[in] keyPressed ASCII code of the key
 * @param[in] mouseX mouse (cursor) X position
 * @param[in] mouseY mouse (cursor) Y position
 */
void keyboardCb(unsigned char keyPressed, int mouseX, int mouseY) {
	switch (keyPressed) {
	case 27:
		glutLeaveMainLoop();
		exit(EXIT_SUCCESS);
	case 'w':
		game_state.keyMap[0] = true;
		break;
	case 'a':
		game_state.keyMap[1] = true;
		break;
	case 's':
		game_state.keyMap[2] = true;
		break;
	case 'd':
		game_state.keyMap[3] = true;
		break;
	case 'r':
		restart();
		break;
	case 'f':
		game_state.isFogActive = !game_state.isFogActive;
		break;
	}
}

/**
 * @brief Initialize application data and OpenGL stuff.
 */
void initApplication() {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	loadShaderPrograms();
	restart();
	initModels();
}

/**
 * @brief Delete all OpenGL objects and application data.
 */
void finalizeApplication(void) {
	cleanupModels();
	cleanUpObjects();
	cleanupShaderPrograms();
}


/**
 * @brief Entry point of the application.
 * @param argc number of command line arguments
 * @param argv array with argument strings
 * @return 0 if OK, <> elsewhere
 */
int main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitContextVersion(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);

		glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
		glutCreateWindow(WINDOW_TITLE);

		glutDisplayFunc(displayCb);
		glutReshapeFunc(reshapeCb);
		glutKeyboardFunc(keyboardCb);
	    glutKeyboardUpFunc(keyboardUpCb);
		glutSpecialFunc(specialKeyboardCb);
		glutMouseFunc(mouseCb);
		glutTimerFunc(33, timerCb, 0);
		if (!pgr::initialize(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR))
			pgr::dieWithError("pgr init failed, required OpenGL not supported?");
	initApplication();

	// handle window close by the user
	glutCloseFunc(finalizeApplication);

	// Infinite loop handling the events
	glutMainLoop();

	return EXIT_SUCCESS;
}
