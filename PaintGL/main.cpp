#include <stdio.h>
#include <iostream>

#include "MeGLWindow.h"
#include <GLFW/glfw3.h>
#include "Tipsify.h"
#include "objLoader/ObjLoader.h"
#include "Ours.h"

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

// Edit later
float *** new_Array3D(int x, int y, int z)
{
	float *** array3D = new float** [x];
	for (int i = 0; i < x; i++)
	{
		array3D[i] = new float* [y];
		for (int j = 0; j < y; j++)
		{
			array3D[i][j] = new float[z];
		}
	}
	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			for (int k = 0; k < z; k++)
			{
				array3D[i][j][k] = 0.0f;
			}
		}
	}

	return array3D;
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
		//moveCenter(pfFramesVertexPositionsIn[frameId], vertexCount);		
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
void preProcessing()
{
	/* load data */

	/* linear vertex-locality patches*/
}
int main(int argc, char *argv[])
{

	int numClusters = 2; int numViews = 4; int iCacheSize = 20; float lamda = 0.85;
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
	
	/* Generate linear face index and Generate vertex-locality patches 
	   return centered vertices positions and patch positions , linear face, piPatchesOut */
	int * piIndexBufferOut = new int[numFaces * 3];
	int * piPatchesOut = new int[numFaces];
	int *piScratch = NULL;
	/* linear clustering to get vertex-locality patches */
	int * piPatchesTmp = new int[numFaces]; int tempNumPatches = 0;
	FanVertOptimizeVCacheOnly(piIndexBufferIn, piIndexBufferOut, numVertices, numFaces, iCacheSize, piScratch, piPatchesTmp, &numPatches);
	delete[] piIndexBufferIn; // delete after the v-cache optimization
	printf("cache only # of patches: %u\n", numPatches);
	printf("cluster start at %u , end at %u \n", piPatchesTmp[0], piPatchesTmp[numPatches]);
	FanVertOptimizeClusterOnly(piIndexBufferOut, numVertices, numFaces, iCacheSize, lamda, piPatchesTmp, numPatches, piPatchesOut, &numPatches, piScratch);
	delete[] piPatchesTmp;
	printf("lamda set # of patches: %u\n", numPatches);
	printf("cluster start at %u , end at %u \n", piPatchesOut[0], piPatchesOut[numPatches]);
	
	/* ours algorithm */
	// move the mesh to center(0,0,0)
	ours animationTest;
//	int viewIds[5] = { 148, 54, 17, 92, 45 };
	int viewIds[5] = { 0,2};
	
	Vector **pvFramesPatchesPositions = new_Array2D<Vector>(numFrames, numPatches);
	int ** means = new_Array2D<int>(numClusters, numFaces * 3);
	int ** assignments = new_Array2D<int>(numFrames, numViews);
	float ** minRatios = new_Array2D<float>(numFrames, numViews);

	animationTest.setParameter(viewIds, numClusters, numFrames, numFaces, numVertices, numPatches,numViews);
	for (int i = 0; i < numFrames; i++)
	{
		animationTest.computePatchPos(piIndexBufferOut, pfFramesVertexPositionsIn[i], piPatchesOut, pvFramesPatchesPositions[i]);
	}
	animationTest.initMeans(means, pvFramesPatchesPositions, piIndexBufferOut, piPatchesOut, pfCameraPositions);

	// read back overdraw ratio
	//test 3d array
	float ***allRatio = new_Array3D(numFrames, numClusters, numViews);

	/* play with our obj Files */
	MeGLWindow meWindow;
	meWindow.setOffScreen(true);
	meWindow.setWindowProperty(450, 450,numViews);
	meWindow.initializeGL();
	meWindow.paintParameter();
	
	// ?? why we need to first load geometry before loading camera matrix
	meWindow.loadGeo(pfFramesVertexPositionsIn[0], numVertices, piIndexBufferOut, numFaces);
	meWindow.setCamera(pfCameraPositions, numViews);

	/* get all ratios */
	for (int i = 0; i < numFrames; i++)
	{
		for (int j = 0; j < numClusters; j++)
		{
			printf(" cluster id: %u\n", j);
			meWindow.loadGeo(pfFramesVertexPositionsIn[i], numVertices, means[j], numFaces);
			meWindow.render(numViews);
			meWindow.overdrawRatio(allRatio[i][j]);
			meWindow.showGL();

		}
	}
	meWindow.teminateGL();

	// make assignments
	float updateRatio;
	updateRatio = animationTest.makeAssignment(assignments, minRatios, allRatio);
	printf("updateRatio: %f\n", updateRatio);

	// new cluster mean
	Vector *pvCameraPositions = (Vector*)pfCameraPositions;
	int * tempMean = new int[numFaces * 3];

	memset(tempMean, 0, numFaces * 3 * sizeof(int));
	animationTest.newClusterMean(piIndexBufferOut, piPatchesOut, pvFramesPatchesPositions, pvCameraPositions, assignments, 0, tempMean);
	// compare which mean to use
	
	// iteration 

	// output results
	delete[] tempMean;

	

	
	/* run our animation methods*/
	



	delete_Array2D(pfFramesVertexPositionsIn, numFrames, numVertices);
	delete_Array2D(pvFramesPatchesPositions, numFrames, numPatches);
	delete [] piIndexBufferOut;
	delete[] piPatchesOut;
	delete[] pfCameraPositions;
	delete_Array2D(means, numClusters, numFaces * 3);
	delete_Array2D(assignments, numFrames, numViews);
	delete_Array2D(minRatios, numFrames, numViews);

	getchar();
	return 1;
	
}