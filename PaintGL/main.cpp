#include <stdio.h>
#include <iostream>
#include <fstream>
#include <ctime>

#include "MeGLWindow.h"
#include <GLFW/glfw3.h>
#include "Tipsify.h"
#include "objLoader/ObjLoader.h"
#include "Ours.h"

#define SCREEN_SIZE 400
#define OFF_SCREEN true
#define  MAXIteration  10
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

void OnError(int errorCode, const char* msg) {
	throw std::runtime_error(msg);
}

int main(int argc, char *argv[])
{
	int numClusters = 5; int numViews = 162; int iCacheSize = 20; float lamda = 0.85;
	int aniDuration[6] = { 75, 50, 70, 50, 45, 40 };
	int viewIds[5] = { 148, 54, 17, 92, 45 };

	int characterId, aniId;
	characterId = atoi(argv[1]);
	aniId = atoi(argv[2]);
	//characterId = 1; aniId = 0;
	printf("charcterIndex: %u, aniId: %u\n", characterId, aniId);
	std::fstream outfile("loop-subdiv1-results.txt", std::ofstream::app);
	outfile << "character: " << characterId << " Animation: " << aniId << std::endl;
	outfile << "viewIds: ";
	for (int i = 0; i < numClusters; i++)
		outfile << viewIds[i]<<"  ";
	outfile << std::endl;

	int numVertices, numFaces, numFrames, numPatches;
	/*set frame select folder*/
	char objFolder[150];
	setFolderPath(objFolder, characterId, aniId);
	/* set animation length*/
	numFrames = aniDuration[aniId];
	
	/* get basic information to allocate memory*/
	getBasicInformation(objFolder, &numFaces, &numVertices);
	printf("%s \n", objFolder);
	printf("# of vertice: %u\n", numVertices);
	printf("# of faces: %u\n", numFaces);
	outfile << "# of vertice: " << numVertices << std::endl;
	outfile << "# of faces  " << numFaces << std::endl;

	/* allocate memory*/
	float **pfFramesVertexPositionsIn = new_Array2D<float>(numFrames, numVertices * 3);
	int *piIndexBufferIn = new int[numFaces * 3];
	float *pfCameraPositions = new float[numViews*3];
	loadData(piIndexBufferIn, pfFramesVertexPositionsIn, numVertices, numFaces, objFolder, numFrames);
	/* change later might use min ball method */
	loadCameras(characterId, aniId, pfCameraPositions, numViews);
	//printf("camera 0 position: %f ,%f ,%f \n", pfCameraPositions[0], pfCameraPositions[1], pfCameraPositions[2]);

	/* Generate linear face index and Generate vertex-locality patches 
	   return centered vertice positions and patch positions , linear face, piPatchesOut */
	int * piIndexBufferOut = new int[numFaces * 3];
	int * piPatchesOut = new int[numFaces];
	/* linear clustering to get vertex-locality patches */
	int * piPatchesTmp = new int[numFaces]; int *piScratch = NULL;
	FanVertOptimizeVCacheOnly(piIndexBufferIn, piIndexBufferOut, numVertices, numFaces, iCacheSize, piScratch, piPatchesTmp, &numPatches);
	printf("cache only # of patches: %u\n", numPatches);
	FanVertOptimizeClusterOnly(piIndexBufferOut, numVertices, numFaces, iCacheSize, lamda, piPatchesTmp, numPatches, piPatchesOut, &numPatches, piScratch);
	printf("lamda set # of patches: %u\n", numPatches);
	outfile << "# of patches: " << numPatches << std::endl;

	/* ours algorithm */
	ours animationTest;

	Vector **pvFramesPatchesPositions = new_Array2D<Vector>(numFrames, numPatches);
	int ** means = new_Array2D<int>(numClusters, numFaces * 3);
	int ** assignments = new_Array2D<int>(numFrames, numViews);
	float ** minRatios = new_Array2D<float>(numFrames, numViews);
	Vector *pvCameraPositions = (Vector*)pfCameraPositions;
	int * tempMean = new int[numFaces * 3];
	float ***allRatio = new_Array3D(numFrames, numClusters, numViews);

	time_t tstart, tend;
	tstart = time(0);
	animationTest.setParameter(viewIds, numClusters, numFrames, numFaces, numVertices, numPatches,numViews);
	for (int i = 0; i < numFrames; i++)
	{
		animationTest.computePatchPos(piIndexBufferOut, pfFramesVertexPositionsIn[i], piPatchesOut, pvFramesPatchesPositions[i]);
	}
	animationTest.initMeans(means, pvFramesPatchesPositions, piIndexBufferOut, piPatchesOut, pfCameraPositions);

	/* initial window and setup parameter */
	MeGLWindow meWindow;
	meWindow.setOffScreen(OFF_SCREEN);
	meWindow.setWindowProperty(SCREEN_SIZE, SCREEN_SIZE,numViews);
	meWindow.initializeGL();
	meWindow.paintParameter();
	meWindow.setBufferObject( numVertices,  numFaces,numViews);
	meWindow.iniAtomicBuffer();

	outfile << "updateRatio: ";
	float *updateRatios = new float[MAXIteration];
	bool move = false; bool tempMove; int iter;
	for (iter = 0; iter < MAXIteration;iter++)
	{
		printf("\n");
		printf("%u iteration \n", iter);
		/* read all the overdraw ratio */
		meWindow.setCamera(pfCameraPositions, numViews);
		for (int i = 0; i < numFrames; i++)
		{
			for (int j = 0; j < numClusters; j++)
			{
				//printf("frame id : %u , cluster id: %u\n",i, j);
				meWindow.subLoadGeo(pfFramesVertexPositionsIn[i], numVertices, means[j], numFaces);
				meWindow.render(numViews);
				// read back overdrawRatio because the screen size is quite large so make it half half
				meWindow.overdrawRatio(allRatio[i][j]);
				meWindow.showGL();
			}
		}
		// make assignments
		updateRatios[iter] = animationTest.makeAssignment(assignments, minRatios, allRatio);
		printf("\n");
		printf("updateRatio: %f\n", updateRatios[iter]);
		outfile << " " << updateRatios[iter];
		move = false;
		/* update each cluster index buffer*/
		for (int clusterId = 0; clusterId < numClusters; clusterId++)
		{
			//printf("update index buffer for cluster %u \n", clusterId);
			memset(tempMean, 0, numFaces * 3 * sizeof(int));
			animationTest.newClusterMean(piIndexBufferOut, piPatchesOut, pvFramesPatchesPositions, pvCameraPositions, assignments, clusterId, tempMean);
			tempMove = compareClusterBuffer(clusterId, means, tempMean, assignments, meWindow, pfCameraPositions, pfFramesVertexPositionsIn, numFrames, numViews, numVertices, numFaces);
			if (tempMove == true)
				move = true;
		}
		if (move == false)
		{
			printf("\n");
			printf("no cluster index buffer changes, finish iteration! \n");
			break;
		}
	}
	tend = time(0);
	outfile << "\n It took" << difftime(tend, tstart) << "second(s) to finish " << (iter + 1) << " times iteration " << std::endl;
	printf("iter: %u\n", iter);
	meWindow.teminateGL();

	// output results
	delete[] tempMean;
	delete[] piPatchesTmp;
	delete[] piIndexBufferIn; // delete after the v-cache optimization
	delete_Array2D(pfFramesVertexPositionsIn, numFrames, numVertices);
	delete_Array2D(pvFramesPatchesPositions, numFrames, numPatches);
	delete [] piIndexBufferOut;
	delete[] piPatchesOut;
	delete[] pfCameraPositions;
	delete_Array2D(means, numClusters, numFaces * 3);
	delete_Array2D(assignments, numFrames, numViews);
	delete_Array2D(minRatios, numFrames, numViews);

	return 1;
	
}