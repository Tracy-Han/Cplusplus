#include <stdio.h>
#include <iostream>

#include "MeGLWindow.h"
#include <GLFW/glfw3.h>
#include "Tipsify.h"
#include "objLoader/ObjLoader.h"

int test()
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(480, 320, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Draw a triangle */
		glBegin(GL_TRIANGLES);

		glColor3f(1.0, 0.0, 0.0);    // Red
		glVertex3f(0.0, 1.0, 0.0);

		glColor3f(0.0, 1.0, 0.0);    // Green
		glVertex3f(-1.0, -1.0, 0.0);

		glColor3f(0.0, 0.0, 1.0);    // Blue
		glVertex3f(1.0, -1.0, 0.0);

		glEnd();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
}

void printVector(obj_vector *v)
{
	printf("%.2f,", v->v[0]);
	printf("%.2f,", v->v[1]);
	printf("%.2f  \n", v->v[2]);
}

void moveCenter(float * pfVertexPositions, int vertexCount)
{
	float center[3] = { 0, 0, 0 };
	for (int vertexId = 0; vertexId < vertexCount; vertexId++)
	{
		center[0] += pfVertexPositions[vertexId * 3];
		center[1] += pfVertexPositions[vertexId * 3 + 1];
		center[2] += pfVertexPositions[vertexId * 3 + 2];
	}
	center[0] /= vertexCount;
	center[1] /= vertexCount;
	center[2] /= vertexCount;

	for (int vertexId = 0; vertexId < vertexCount; vertexId++)
	{
		pfVertexPositions[vertexId * 3] -= center[0];
		pfVertexPositions[vertexId * 3 + 1] -= center[1];
		pfVertexPositions[vertexId * 3 + 2] -= center[2];
	}
}
void loadData(int * piIndexBuffer, float ** pfFramesVertexPositionsIn, int *numFaces, int *numVertices, char * objFolder, int duration)
{
	char filePath[100];
	int i = 0; char buffer[10];
	strcpy(filePath, objFolder);
	strcat(filePath, "frame");
	itoa(i + 1, buffer, 10);
	strcat(filePath, buffer);
	strcat(filePath, ".obj");
	printf(filePath); printf("\n");

	objLoader *testLoader = new objLoader();
	testLoader->load(filePath);

	int vertexCount = testLoader->vertexCount;
	int faceCount = testLoader->faceCount;
	
	*numFaces = faceCount;
	*numVertices = vertexCount;

	int count = 0;
	for (int i = 0; i < faceCount; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			piIndexBuffer[count] = testLoader->faceList[i]->vertex_index[j];
			count++;
		}
	}
	

	for (int frameId = 0; frameId < duration; frameId++)
	{
		char filePath[100];
		char buffer[10];
		strcpy(filePath, objFolder);
		strcat(filePath, "frame");
		itoa(frameId + 1, buffer, 10);
		strcat(filePath, buffer);
		strcat(filePath, ".obj");
		printf(filePath); printf("\n");

		objLoader *Loader = new objLoader();
		Loader->load(filePath);

		int count = 0;
		for (int vertexId = 0; vertexId < vertexCount; vertexId++)
		{
			for (int j = 0; j < 3; j++)
			{
				pfFramesVertexPositionsIn[frameId][count] = Loader->vertexList[vertexId]->v[j];
				count++;
			}
		}
		moveCenter(pfFramesVertexPositionsIn[frameId], vertexCount);
		
	}

}

void loadCameras(char * character,char * animation,float * pfCameraPositions,int numViews)
{
	char vfFolder[150]; char cameraPath[150];
	strcpy(vfFolder, "D:/TriangleOrdering/Temp/Animation/VerticeFace/");
	/*strcat(vfFolder, Character[characterId]);*/
	strcat(vfFolder, character);
	strcat(vfFolder, "/");
	strcat(vfFolder, animation);
	strcat(vfFolder, "/");
	FILE * myFile;
	strcpy(cameraPath, vfFolder);
	strcat(cameraPath, "newViewpoint3.txt");
	//strcat(cameraPath, "rotateView3.txt");
	printf(cameraPath);
	printf("\n");

	myFile = fopen(cameraPath, "r");
	for (int i = 0; i < numViews * 3; i++)
	{
		fscanf(myFile, "%f \n", &pfCameraPositions[i]);
	}
	fclose(myFile);
}

void OnError(int errorCode, const char* msg) {
	throw std::runtime_error(msg);
}

void test2()
{
	glfwSetErrorCallback(OnError);
	glfwInit();
	if (!glfwInit())
		throw std::runtime_error("glfwInit failed");

	GLFWwindow* gWindow;
	gWindow = glfwCreateWindow(500,500, "openglTutrials", NULL, NULL);

	if (!gWindow)
		throw std::runtime_error("glfwCreateWindow failed. Can your hardware handle OpenGL 4.5?");
	glfwMakeContextCurrent(gWindow);

	// initialize GLEW
	glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "ERROR: %s\n", glewGetErrorString(err));
		//throw std::runtime_error("glewInit failed");
	}

	GLfloat verts[] =
	{
		+0.0f, +1.0f,
		-1.0f, -1.0f,
		+1.0f, -1.0f,
	};
	GLuint myBufferID;
	glGenBuffers(1, &myBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, myBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	while (!glfwWindowShouldClose(gWindow))
	{
		//glBindVertexArray(myBufferID);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwSwapBuffers(gWindow);
		glfwPollEvents();
	}

	glfwTerminate();

}
int main(void)
{
	
	MeGLWindow meWindow;
	meWindow.setOffScreen(false);
	meWindow.setWindowProperty(500, 500);
	meWindow.setCameraPosition(glm::vec3(4.0f, 4.0f, 3.0f));
	meWindow.paintGL();
	
	
	return 1;
	
}