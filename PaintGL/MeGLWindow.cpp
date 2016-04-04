#include "MeGLWindow.h"
#include <iostream>
#include <fstream>
#include <glm\glm.hpp>
#include "primitives\vertex.h"
#include "primitives\shapeGenerator.h"


GLuint numIndices;

using glm::mat4;
const int NUM_FLOATS_PER_VERTEX = 9;
const int VERTEX_BYTE_SIZ = NUM_FLOATS_PER_VERTEX*sizeof(float);
void MeGLWindow::initializeGL()
{

}

void MeGLWindow::setOffScreen(bool off)
{
	offScreen = off;
}
void MeGLWindow::setWindowProperty(int x, int y)
{
	width = x;
	height = y;
}
void MeGLWindow::setCameraPosition(glm::vec3 testPos)
{
	position = testPos;
}
void MeGLWindow::setCamera()
{
	/*Camera gCamera;*/
	gCamera.setPosition(position);
	gCamera.setViewportAspectRatio(1.0f*width / height);
	gCamera.setFieldOfView(60.0f);
	gCamera.setNearAndFarPlanes(0.1f, 2000.0f);
	gCamera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

	mat4 fullTransformMatrix = glm::translate(glm::mat4(), glm::vec3(-0.5, 0.5, 0.0)) * glm::scale(glm::mat4(1.0), glm::vec3(1.0 / 2, 1.0 / 2, 1.0)) * gCamera.matrix();
//	GLint fullTransformMatrixLocation = glGetUniformLocation(programID, "fullTransformMatrix");
//	glUniformMatrix4fv(fullTransformMatrixLocation, 1, GL_FALSE, &fullTransformMatrix[0][0]);
	
	glGenBuffers(1, &transformMatrixBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, transformMatrixBufferID);
	GLuint transformMatrixLocation = glGetAttribLocation(programID,"fullTransformMatrix");
	GLuint pos0 = transformMatrixLocation + 0;
	GLuint pos1 = transformMatrixLocation + 1;
	GLuint pos2 = transformMatrixLocation + 2;
	GLuint pos3 = transformMatrixLocation + 3;

	glBufferData(GL_ARRAY_BUFFER, sizeof(fullTransformMatrix), &fullTransformMatrix, GL_STATIC_DRAW);
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

	glBindBuffer(GL_VERTEX_ARRAY,0);
	glBindVertexArray(0);
}
void MeGLWindow::loadGeo()
{
//	shapeData myShape = shapeGenerator::makeTriangles();
	shapeData myShape = shapeGenerator::makeCube();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, myShape.vertexBufferSize(), myShape.vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3 , GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZ, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZ, (const GLvoid*)(sizeof(GLfloat)*3));

	glGenBuffers(1, &indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, myShape.indicesBufferSize(), myShape.indices, GL_STATIC_DRAW);
	numIndices = myShape.numIndices;
	myShape.cleanup();
	
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
void MeGLWindow::overdrawRatio()
{
	unsigned char* pixel = new unsigned char[width*height];
	memset(pixel, 0, width*height*sizeof(unsigned char));

	if (offScreen)
	{
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER,fbo);
	}
	else
	{
		glReadBuffer(GL_BACK);
	}
	glReadPixels(0, 0, width, height, GL_RED,GL_UNSIGNED_BYTE,pixel);

	int x, y;
	float averagePixel = 0.0f;;
	float showedPixel = 0.0f;
	for (int i = 0; i < height; i++) //height
	{
		for (int j = 0; j < width; j++)//width
		{
			if ((int)pixel[i * width + j] > 0)
			{
				averagePixel += (float)pixel[i*width + j];
				showedPixel += 1.0f;
			}
		}
	}
	averagePixel =averagePixel/showedPixel;
//	std::cout << averagePixel << std::endl;
	delete[] pixel;
}
void MeGLWindow::render()
{
	if (offScreen)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,fbo);
	}

	glClearColor(0, 0, 0, 1); // black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawElements(GL_TRIANGLES,
		numIndices, GL_UNSIGNED_SHORT, NULL);

	//overdrawRatio();
}
int MeGLWindow::paintGL()
{
	/* initialize window */
	if (!glfwInit())
		return -1;
	GLFWwindow* window;
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

	/* set fbo*/

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
	/* load geometry*/
	loadGeo();
	
	/*load shader*/
	installShaders();

	/* set uniform*/
	setCamera();

	glViewport(0, 0, width, height);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glBindVertexArray(VAO);
	/* draw geometry*/
	while (!glfwWindowShouldClose(window))
	{
		// drawing contents
		render();

		glfwSwapBuffers(window);

		glfwPollEvents();
	}
	
	/* finish drawing*/
	glfwTerminate();
	return 1;
}