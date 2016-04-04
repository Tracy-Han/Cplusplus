#include <stdio.h>
#include <iostream>

#include "MeGLWindow.h"
#include <GLFW/glfw3.h>
#include "Tipsify.h"
#include "objLoader/ObjLoader.h"

// malloc 2 dimension array
template <typename T>
T** new_Array2D(int row, int col)
{
	int size = sizeof(T);
	int point_size = sizeof(T*);
	//
	T **arr = (T **)malloc(point_size * row + size * row * col);
	if (arr != NULL)
	{
		T *head = (T*)((int)arr + point_size * row);
		for (int i = 0; i < row; ++i)
		{
			arr[i] = (T*)((int)head + i * col * size);
			for (int j = 0; j < col; ++j)
				new (&arr[i][j]) T;
		}
	}
	return (T**)arr;
}
//release
template <typename T>
void delete_Array2D(T **arr, int row, int col)
{
	for (int i = 0; i < row; ++i)
		for (int j = 0; j < col; ++j)
			arr[i][j].~T();
	if (arr != NULL)
		free((void**)arr);
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
void loadData(int * piIndexBuffer, float ** pfFramesVertexPositionsIn, int vertexCount, int faceCount, char* objFolder, int duration)
{
	bool readFace = false;

	char filePath[100]; char buffer[10];
	for (int frameId = 0; frameId < duration; frameId++)
	{
		
		strcpy(filePath, objFolder);
		strcat(filePath, "frame");
		itoa(frameId + 1, buffer, 10);
		strcat(filePath, buffer);
		strcat(filePath, ".obj");
		//printf("%s \n",filePath);

		objLoader *Loader = new objLoader();
		Loader->load(filePath);

		if (!readFace)
		{
			//printf("read face indices \n");
			int count = 0;
			for (int i = 0; i < faceCount; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					piIndexBuffer[count] = Loader->faceList[i]->vertex_index[j];
					count++;
				}
			}
			readFace = true;
		}

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

void loadCameras(int characterId,int aniId,float *pfCameraPositions,int numViews)
{
	char Character[5][20] = { "Ganfaul_M_Aure", "Kachujin_G_Rosales", "Maw_J_Laygo", "Nightshade" };
	char Animation[6][40] = { "dancing_maraschino_step", "Standing_2H_Cast_Spell", "Standing_2H_Magic_Area_Attack", "Standing_Jump", "Standing_React_Death_Backward", "Standing_React_Large_From_Back" };


	char cameraPath[150];
	strcpy(cameraPath, "D:/TriangleOrdering/Temp/Animation/VerticeFace/");
	strcat(cameraPath, Character[characterId]);
	strcat(cameraPath, "/");
	strcat(cameraPath, Animation[aniId]);
	strcat(cameraPath, "/");
	strcat(cameraPath, "newViewpoint3.txt");
//	printf("%s \n",cameraPath);

	FILE * myFile;
	myFile = fopen(cameraPath, "r");
	for (int i = 0; i < numViews*3; i++)
	{
			fscanf(myFile, "%f \n", &pfCameraPositions[i]);
	}
	fclose(myFile);
}

void OnError(int errorCode, const char* msg) {
	throw std::runtime_error(msg);
}

//void test()
//{
//	MeGLWindow meWindow;
//	meWindow.setOffScreen(false);
//	meWindow.setWindowProperty(500, 500);
//	meWindow.setCameraPosition(glm::vec3(4.0f, 4.0f, 3.0f));
//	meWindow.paintGL();
//
//}
void getBasicInformation(char * objFolder, int* numFaces, int* numVertices)
{
	char objFile[150];
	strcpy(objFile, objFolder);
	strcat(objFile, "frame1.obj");
	

	objLoader *testLoader = new objLoader();
	testLoader->load(objFile);

	*numVertices = testLoader->vertexCount;
	*numFaces = testLoader->faceCount;
	
}
void setFolderPath(char *objFolder, int characterId,int aniId)
{
	char Character[5][20] = { "Ganfaul_M_Aure", "Kachujin_G_Rosales", "Maw_J_Laygo", "Nightshade" };
	char Animation[6][40] = { "dancing_maraschino_step", "Standing_2H_Cast_Spell", "Standing_2H_Magic_Area_Attack", "Standing_Jump", "Standing_React_Death_Backward", "Standing_React_Large_From_Back" };

	strcpy(objFolder, "D:/TriangleOrdering/VF/Obj/");
	strcat(objFolder, Character[characterId]);
	strcat(objFolder, "/");
	strcat(objFolder, Animation[aniId]);
	strcat(objFolder, "/nosub/");
}
int main(int argc, char *argv[])
{

	int numClusters = 5; int numViews = 10; int iCacheSize = 20; float lamda = 0.85;
	int characterId,aniId;
	characterId = 1; aniId = 0;

	int numVertices, numFaces, numFrames,numPatches;

	/*set frame select folder*/
	char objFolder[150];
	setFolderPath(objFolder, characterId, aniId);
	/* set animation length*/
	int aniDuration[6] = { 5, 50, 70, 50, 45, 40 };
	numFrames = aniDuration[aniId];
	
	/* get basic information to allocate memory*/
	getBasicInformation(objFolder, &numFaces, &numVertices);
	printf("%s \n", objFolder);
	printf("# of vertices: %u\n", numVertices);
	printf("# of faces: %u\n", numFaces);

	/* allocate memory*/
	float **pfFramesVertexPositionsIn = new_Array2D<float>(numFrames, numVertices * 3);
	int *piIndexBufferIn = new int[numFaces * 3];
	float *pfCameraPositions = new float[numViews*3];
	loadData(piIndexBufferIn, pfFramesVertexPositionsIn, numVertices, numFaces, objFolder, numFrames);

	/* change later might use min ball method */
	loadCameras(characterId, aniId, pfCameraPositions, numViews);
	printf(" fist camera position %f , %f , %f \n", pfCameraPositions[0], pfCameraPositions[1], pfCameraPositions[2]);
	//printf(" 162 camera position %f , %f , %f \n", pfCameraPositions[numViews*3-3], pfCameraPositions[numViews*3-2], pfCameraPositions[numViews*3-1]);
	
	/* Generate patches */
	int * piIndexBufferOut = new int[numFaces * 3];
	int * piClustersOut = new int[numFaces];
	int *piScratch = NULL;
	/* linear clustering to get vertex-locality patches */
	int * piClustersTmp = new int[numFaces];
	FanVertOptimizeVCacheOnly(piIndexBufferIn,piIndexBufferOut,numVertices,numFaces,iCacheSize,piScratch,piClustersTmp,&numPatches);
	delete[] piIndexBufferIn; // delete after the v-cache optimization
	printf("# of patches: %u\n", numPatches);
	printf("cluster start at %u , end at %u \n", piClustersTmp[0], piClustersTmp[numPatches]);
	FanVertOptimizeClusterOnly(piIndexBufferOut, numVertices, numFaces, iCacheSize, lamda, piClustersTmp, numPatches, piClustersOut, &numPatches, piScratch);
	delete[] piClustersTmp;
	printf("# of patches: %u\n", numPatches);
	printf("cluster start at %u , end at %u \n", piClustersOut[0], piClustersOut[numPatches]);



	/* play with our obj Files */
	MeGLWindow meWindow;
	meWindow.setOffScreen(false);
	meWindow.setWindowProperty(800, 800);
//	meWindow.setCameraPosition(glm::vec3(pfCameraPositions[0], pfCameraPositions[1], pfCameraPositions[2]));
	meWindow.paintGL(pfFramesVertexPositionsIn[0],numVertices,piIndexBufferOut,numFaces,pfCameraPositions,numViews);


	int ** means = new_Array2D<int>(numClusters, numFaces * 3);
	int ** assignments = new_Array2D<int>(numFrames, numViews);
	float ** minRatios = new_Array2D<float>(numFrames, numViews);
	/* run our animation methods*/
	



	delete_Array2D(pfFramesVertexPositionsIn, numFrames, numVertices);
	delete [] piIndexBufferOut;
	delete[] piClustersOut;
	delete[] pfCameraPositions;
	delete_Array2D(means, numClusters, numFaces * 3);
	delete_Array2D(assignments, numFrames, numViews);
	delete_Array2D(minRatios, numFrames, numViews);

	getchar();
	return 1;
	
}