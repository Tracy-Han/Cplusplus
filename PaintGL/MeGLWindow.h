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
	int numSlices;
	bool offScreen;
	bool readOVR;
	bool readAtomic;
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
	GLuint baseInstance;
	GLFWwindow* window;

public:
	void initializeGL();
	void setWindowProperty(int x, int y);
	void setOffScreen(bool off);
	void setReadOVR(bool readOVR);
	void setReadAtomic(bool readAtomic);
	void setCameraPosition(glm::vec3 pose);
	void setCamera(float* pfCameraPositions, int numViews);
	void loadGeo(float* pfVertexPositions, int numVertices, int* piIndexBuffer, int numFaces);
	void installShaders();
	void overdrawRatio();
	void render(int numViews);
	int paintGL(float* pfVertexPositions, int numVertices, int* piIndexBuffer, int numFaces,float* pfCameraPositions,int numViews);
	void teminateGL();
};


#endif

