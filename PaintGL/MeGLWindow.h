#ifndef MEGLWINDOW_H
#define MEGLWINDOW_H

#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include "camera\Camera.h"
#include <glm\gtc\matrix_transform.hpp>
#include <stdexcept>
class MeGLWindow
{
protected:
	int width;
	int height;
	int canvasX;
	int canvasY;
	bool offScreen;
	GLuint fbo;
	GLuint gColor;
	GLuint rboDepth;
	GLuint VAO;
	GLuint VBO;
	GLuint transformMatrixBufferID;
	GLuint indexBufferID;
	GLuint programID;
	Camera gCamera;
	glm::vec3 position;

public:
	void initializeGL();
	void setWindowProperty(int x, int y);
	void setOffScreen(bool off);
	void setCameraPosition(glm::vec3 pose);
	void setCamera();
	void loadGeo();
	void installShaders();
	void overdrawRatio();
	void render();
	int paintGL();
};


#endif

