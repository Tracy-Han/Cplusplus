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
	GLuint atomicBufferID;

public:
	void initializeGL();
	void setWindowProperty(int x, int y,int numViews);
	void setOffScreen(bool off);
	void setReadOVR(bool readOVR);
	void setReadAtomic(bool readAtomic);
	void setCamera(float* pfCameraPositions, int numViews);
	// This still set a big canvas might change later
	// 1. small canvas
	void setClusterCamera(float* pfCameraPositions, int numViews, int ** assignments, int frameId, int clusterId);
	void setBufferObject(float* pfVertexPositions, int numVertices, int* piIndexBuffer, int numFaces,int numViews);
	void subLoadGeo(float* pfVertexPositions, int numVertices, int* piIndexBuffer, int numFaces);
	void installShaders();
	void overdrawRatio(float *sliceRatio);
	void render(int numViews);
	int paintParameter();
	void showGL();
	void teminateGL();
	void iniAtomicBuffer();
	void setZeroAtomicBuffer();
	GLuint readAtomicBuffer();
};


#endif

