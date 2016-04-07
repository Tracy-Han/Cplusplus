#include "MeGLWindow.h"
#include <iostream>
#include <fstream>
#include <glm\glm.hpp>
#include "primitives\vertex.h"
#include "primitives\shapeGenerator.h"


GLuint numIndices;
/* please check the vertex and index  -->  type and size*/
using glm::mat4;
const int NUM_FLOATS_PER_VERTEX = 3;
const int VERTEX_BYTE_SIZ = NUM_FLOATS_PER_VERTEX*sizeof(float);
void MeGLWindow::initializeGL()
{
	/* initialize window */
	if (!glfwInit())
		return ;
//	GLFWwindow* window;
	if (offScreen)
	{
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
		window = glfwCreateWindow(width, height, "Hello World", NULL, NULL);
		glfwHideWindow(window);
	}
	else
	{
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		window = glfwCreateWindow(width, height, "Hello World", NULL, NULL);

	}
	if (!window)
	{
		glfwTerminate();
		throw std::runtime_error("glfwCreateWindow failed. Can your hardware handle OpenGL 4.3?");
	}
	glfwMakeContextCurrent(window);

	/* initialize glew */
	glewInit();

}

void MeGLWindow::setOffScreen(bool off)
{
	offScreen = off;
}
void MeGLWindow::setReadOVR(bool OVRread)
{
	readOVR = OVRread;
}
void MeGLWindow::setReadAtomic(bool atomicRead)
{
	readAtomic = atomicRead;
}
void MeGLWindow::setWindowProperty(int x, int y,int numViews)
{
	int xSlice = ceil(sqrt(numViews));
	width = x * xSlice;
	height = y * xSlice;
}

void MeGLWindow::setCamera(float* pfCameraPositions, int numViews)
{
	/*Camera gCamera;*/
	gCamera.setViewportAspectRatio(1.0f*width / height);
	gCamera.setFieldOfView(45.0f);
	gCamera.setNearAndFarPlanes(0.1f, 2000.0f);

	numSlices = numViews;
	int NumSlice = ceil(sqrt(numViews));
	int sliceX, sliceY;
	//printf("NumSlice: %u\n", NumSlice);
	float *translatePos = new float[NumSlice];
	for (int i = 0; i < NumSlice; i++){
		translatePos[i] = -1 + 1.0 /NumSlice + (2.0 / NumSlice)*i; // original offset plus slot*i 
	}

	mat4 *fullTransformMatrix = new mat4[numViews];
	for (int i = 0; i < numViews; i++)
	{
		sliceX = i % (int)NumSlice;
		sliceY = i / (int)NumSlice;
		gCamera.setPosition(glm::vec3(pfCameraPositions[i * 3], pfCameraPositions[i * 3+1], pfCameraPositions[i * 3+2]));
		gCamera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
		fullTransformMatrix[i] = 
			glm::translate(glm::mat4(), glm::vec3(translatePos[sliceX], translatePos[sliceY], 0.0)) * glm::scale(glm::mat4(1.0), glm::vec3(1.0 / NumSlice, 1.0 / NumSlice, 1.0)) *
			gCamera.matrix();
	}
	delete[] translatePos;

	/* upload camera*/
	glBindBuffer(GL_ARRAY_BUFFER, transformMatrixBufferID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numViews* sizeof(mat4), fullTransformMatrix);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	delete[] fullTransformMatrix;
}
void MeGLWindow::setClusterCamera(float* pfCameraPositions, int numViews, int ** assignments,int frameId ,int clusterId)
{
	/*Camera gCamera;*/
	numSlices = numViews;
	int NumSlice = ceil(sqrt(numViews));
	int sliceX, sliceY;

	float *translatePos = new float[NumSlice];
	for (int i = 0; i < NumSlice; i++){
		translatePos[i] = -1 + 1.0 / NumSlice + (2.0 / NumSlice)*i; // original offset plus slot*i 
	}

	mat4 *fullTransformMatrix = new mat4[numViews];
	memset(fullTransformMatrix, 0.0f, numViews*sizeof(mat4));

	for (int i = 0; i < numViews; i++)
	{
		sliceX = i % (int)NumSlice;
		sliceY = i / (int)NumSlice;
		if (assignments[frameId][i] == clusterId)
		{
			gCamera.setPosition(glm::vec3(pfCameraPositions[i * 3], pfCameraPositions[i * 3 + 1], pfCameraPositions[i * 3 + 2]));
			gCamera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
			fullTransformMatrix[i] =
				glm::translate(glm::mat4(), glm::vec3(translatePos[sliceX], translatePos[sliceY], 0.0)) * glm::scale(glm::mat4(1.0), glm::vec3(1.0 / NumSlice, 1.0 / NumSlice, 1.0)) *
				gCamera.matrix();
		}
	}
	delete[] translatePos;

	glBindBuffer(GL_ARRAY_BUFFER, transformMatrixBufferID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numViews* sizeof(mat4), fullTransformMatrix);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] fullTransformMatrix;
}
void MeGLWindow::setBufferObject( int numVertices,int numFaces,int numViews)
{
	/* vertex object */
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(float), NULL, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3 , GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZ, 0);
	glBindBuffer(GL_ARRAY_BUFFER,0);

	/* camera object */
	glGenBuffers(1, &transformMatrixBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, transformMatrixBufferID);
	GLuint transformMatrixLocation = glGetAttribLocation(programID, "fullTransformMatrix");
	GLuint pos0 = transformMatrixLocation + 0;
	GLuint pos1 = transformMatrixLocation + 1;
	GLuint pos2 = transformMatrixLocation + 2;
	GLuint pos3 = transformMatrixLocation + 3;

	glBufferData(GL_ARRAY_BUFFER, numViews* sizeof(mat4),NULL, GL_STATIC_DRAW);
	glEnableVertexAttribArray(pos0);
	glVertexAttribPointer(pos0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(GLfloat) * 0));
	glEnableVertexAttribArray(pos1);
	glVertexAttribPointer(pos1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(GLfloat) * 4));
	glEnableVertexAttribArray(pos2);
	glVertexAttribPointer(pos2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(GLfloat) * 8));
	glEnableVertexAttribArray(pos3);
	glVertexAttribPointer(pos3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(GLfloat) * 12));

	glVertexAttribDivisor(pos0, 1);
	glVertexAttribDivisor(pos1, 1);
	glVertexAttribDivisor(pos2, 1);
	glVertexAttribDivisor(pos3, 1);

	glBindBuffer(GL_VERTEX_ARRAY, 0);
	glBindVertexArray(0);

	/* index buffer object */
	glGenBuffers(1, &indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numFaces * 3 * sizeof(int), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	numIndices = numFaces * 3;
	
}
void MeGLWindow::subLoadGeo(float* pfVertexPositions, int numVertices, int* piIndexBuffer, int numFaces)
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices * 3 * sizeof(float), pfVertexPositions);
	glBindBuffer(GL_ARRAY_BUFFER,0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,0,numFaces *3 * sizeof(int),piIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void MeGLWindow::iniAtomicBuffer()
{
	glGenBuffers(1, &atomicBufferID);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBufferID);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBufferID);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

}
void MeGLWindow::setZeroAtomicBuffer()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBufferID);
	GLuint a = 0;
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &a);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

}
GLuint MeGLWindow::readAtomicBuffer()
{
	GLuint userCounter;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBufferID);
	glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &userCounter);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
//	printf("redPixels: %u\n", userCounter);
	return userCounter;
}
bool checkShaderStatus(GLuint shaderID)
{
	GLint compileStatus;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus != GL_TRUE)
	{
		GLint infoLogLens;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLens);
		GLchar* buffer = new GLchar[infoLogLens];
		GLsizei bufferSize;
		glGetShaderInfoLog(shaderID, infoLogLens, &bufferSize, buffer);
		std::cout << buffer << std::endl;

		delete[] buffer;
		return false;
	}
	return true;
}
std::string readShaderCode(const char* fileName)
{
	std::ifstream meInput(fileName);
	if (!meInput.good())
	{
		exit(1);
	}

	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>());
		
}
void MeGLWindow::installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const char* adpater[1];
	std::string temp = readShaderCode("vertexShaderCode.glsl");
	adpater[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adpater, 0);
	temp = readShaderCode("fragmentShaderCode.glsl");
	adpater[0] = temp.c_str();
	glShaderSource(fragmentShaderID,1,adpater,0);

	/* how to get error message */
	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);
	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID,vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);



	glUseProgram(programID);
}
void MeGLWindow::overdrawRatio(float *sliceRatio)
{
	printf("width: %u,height %u\n", width, height);

	unsigned char* pixelBuffer = new unsigned char[width*height];
	memset(pixelBuffer, 0, width*height*sizeof(unsigned char));

	if (offScreen)
	{
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER,fbo);
	}
	else
	{
		glReadBuffer(GL_BACK);
	}
	glReadPixels(0, 0, width, height, GL_RED,GL_UNSIGNED_BYTE,pixelBuffer);

	// slice
	int xSlice = ceil(sqrt(numSlices));
	int sliceWidth = width / xSlice;

	int x, y;
	int drawnPixel;int showedPixel=0;
	for (int cameraId = 0; cameraId < numSlices; cameraId++)
	{
		x = cameraId % xSlice; //width
		y = cameraId / xSlice; //height
		drawnPixel = 0; showedPixel = 0;

		for (int i = sliceWidth * y; i < sliceWidth*(y + 1); i++) //height = width
		{
			for (int j = sliceWidth * x; j < sliceWidth*(x + 1); j++)//width
			{
				if ((int)pixelBuffer[i * width + j] > 1)
				{
					drawnPixel += round((float)pixelBuffer[i * width + j] / 51.0f);
					showedPixel++;
				}
			}
		}
		sliceRatio[cameraId]= (float)drawnPixel / (float)showedPixel;
		printf("x : %u ,y : %u , ratio: %f \n", x,y, sliceRatio[cameraId]);
	}
	
	delete[] pixelBuffer;
}

//void MeGLWindow::halfOverdrawRatio(float *sliceRatio)
//{
////	printf("width: %u ,height %u \n", width, height);
//	int xSlice = ceil(sqrt(numSlices));
//	int sliceWidth = width / xSlice;
//	int halfYSlice = ceil(xSlice / 2.0);//xSlice = ySlice
//	int lowerHalfHeight = halfYSlice * sliceWidth;//sliceHeight = sliceWidth
//	int higherHalfHeight = height - lowerHalfHeight;
//	printf("halfYSlice: %u, lower half height %u ,higher half height %u \n", halfYSlice, lowerHalfHeight,higherHalfHeight);
//
//	int pixelSize = width*lowerHalfHeight;
//	unsigned char* pixelBuffer = new unsigned char[pixelSize];
//	memset(pixelBuffer, 0, width*lowerHalfHeight*sizeof(unsigned char));
//	if (offScreen)
//	{
//		glReadBuffer(GL_COLOR_ATTACHMENT0);
//		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
//	}
//	else
//	{
//		glReadBuffer(GL_BACK);
//	}
//
//	int x, y, drawnPixel, showedPixel;
//	printf("width: %u, lowerHalfHeight %u\n", width, lowerHalfHeight);
//	glReadPixels(0, 0, width, lowerHalfHeight, GL_RED, GL_UNSIGNED_BYTE, pixelBuffer);
//	for (int cameraId = 0; cameraId < xSlice*halfYSlice; cameraId++)
//	{
//		x = cameraId % xSlice; //width
//		y = cameraId / xSlice; //height
//		drawnPixel = 0; showedPixel = 0;
//
//		for (int i = sliceWidth * y; i < sliceWidth*(y + 1); i++) //height = width
//		{
//			for (int j = sliceWidth * x; j < sliceWidth*(x + 1); j++)//width
//			{
//				if ((int)pixelBuffer[i * width + j] > 0)
//				{
//					drawnPixel += round((float)pixelBuffer[i * width + j] / 51.0f);
//					showedPixel++;
//				}
//			}
//		}
//		sliceRatio[cameraId] = (float)drawnPixel / (float)showedPixel;
//		printf("x : %u ,y : %u , ratio: %f \n", x,y, sliceRatio[cameraId]);
//	}
//
//	// higher half
//	memset(pixelBuffer, 0, width*lowerHalfHeight*sizeof(unsigned char));
//	printf("lowerHalfHeight: %u, height %u\n", lowerHalfHeight, height);
//	glReadPixels(0, lowerHalfHeight, width,higherHalfHeight, GL_RED, GL_UNSIGNED_BYTE, pixelBuffer);
//	for (int cameraId = xSlice*halfYSlice; cameraId <numSlices; cameraId++)
//	{
//		x = cameraId % xSlice; //width
//		y = cameraId / xSlice - halfYSlice; //height
//		drawnPixel = 0; showedPixel = 0;
//
//		for (int i = sliceWidth * y; i < sliceWidth*(y + 1); i++) //height = width
//		{
//			for (int j = sliceWidth * x; j < sliceWidth*(x + 1); j++)//width
//			{
//				if ((int)pixelBuffer[i * width + j] > 0)
//				{
//					drawnPixel += round((float)pixelBuffer[i * width + j] / 51.0f);
//					showedPixel++;
//				}
//			}
//		}
//		sliceRatio[cameraId] = (float)drawnPixel / (float)showedPixel;
//		printf("x : %u ,y : %u , ratio: %f \n", x,y, sliceRatio[cameraId]);
//	}
//	//printf("all drawn pixel: %u\n", alldrawnPixel);
//
//	//delete[] pixel;
//}
void MeGLWindow::render(int numViews)
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	if (offScreen)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,fbo);
	}

	glClearColor(0, 0, 0, 1); // black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL, numViews);
//	glDrawElementsInstancedBaseInstance(GL_TRIANGLES,numIndices,GL_UNSIGNED_INT,NULL,1,baseInstance);
	
}
int MeGLWindow::paintParameter()
{

	if (offScreen)
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		glGenRenderbuffers(1, &gColor);
		glBindRenderbuffer(GL_RENDERBUFFER, gColor);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RED, width, height);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, gColor);

		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

		GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "frame buffer error:" << std::endl;
			exit(EXIT_FAILURE);
		}

	}
	/*load shader*/
	installShaders();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	///* load geometry*/
	//loadGeo(pfVertexPositions, numVertices, piIndexBuffer, numFaces);

	///* set uniform*/
	//setCamera(pfCameraPositions,numViews);

	glViewport(0, 0, width, height);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
	//glBindVertexArray(VAO); // set it in render
	
	return 1;
}
void MeGLWindow::showGL()
{
	glfwSwapBuffers(window);
	//glfwPollEvents();
}
void MeGLWindow::teminateGL()
{
	/* finish drawing*/
	glfwTerminate();
	printf("terminate window! ");

}