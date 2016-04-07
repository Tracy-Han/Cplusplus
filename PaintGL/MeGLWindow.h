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
	int _width;
	int _height;
	int _numSlices;// numViews
	bool _offScreen;
	bool _readOVR;
	bool _readAtomic;
	GLuint _fbo;
	GLuint _gColor;
	GLuint _rboDepth;
	GLuint _VAO;
	GLuint _VBO;
	GLuint _transformMatrixBufferID;
	GLuint _indexBufferID;
	GLuint _programID;
	Camera _gCamera;
	GLuint _baseInstance;
	GLFWwindow* _window;
	GLuint _atomicBufferID;

public:
	MeGLWindow();
	void initializeGL();
	void setWindowProperty(int x, int y,int numViews);
	void setOffScreen(bool off);
	void setReadOVR(bool readOVR);
	void setReadAtomic(bool readAtomic);
	void setCamera(float* pfCameraPositions, int numViews);
	// This still set a big canvas might change later
	// 1. small canvas
	void setClusterCamera(float* pfCameraPositions, int numViews, int ** assignments, int frameId, int clusterId);
	void setBufferObject( int numVertices, int numFaces,int numViews);
	void subLoadGeo(float* pfVertexPositions, int numVertices, int* piIndexBuffer, int numFaces);
	void installShaders();
	void overdrawRatio(float *sliceRatio);
	//void halfOverdrawRatio(float *sliceRatio);
	void render(int numViews);
	int paintParameter();
	void showGL();
	void teminateGL();
	void iniAtomicBuffer();
	void setZeroAtomicBuffer();
	GLuint readAtomicBuffer();
};


#endif

