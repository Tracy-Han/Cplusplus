#ifndef OURS_H
#define OURS_H

#include <iostream>
#include <algorithm>

#include "objLoader/ObjLoader.h"
#include "vector.h"
#include "MeGLWindow.h"

class ours
{
private:
	int *_pickIds;
	int _numClusters;
	// get from preprocessing
	int _numFrames;
	int _numFaces;
	int _numVertices;
	int _numPatches;
	int _numViews;

public:
	void setParameter(int *viewIds,int Clusters,int framesNum, int facesNum,int verticesNum,int patchesNum,int viewsNum);
	//function that implements getting patches positions
	void computePatchPos(int *piIndexBufferIn,
						 float *pfVertexPositionsIn,
						 int *piClustersIn,
						 Vector * pvPatchesPositions);
	//function that implements sorting and return new faceIndex
	void depthSortPatch(Vector viewpoint, Vector *pvAvgPatchesPositions, int *piIndexBufferIn, int *piClustersIn, int * piIndexBufferTmp);
	void initMeans(int ** means, Vector ** pvFramesPatchesPositions, int * piIndexBufferIn, int * piClustersIn, float * pfCameraPositions);
	//function that assign (frame,view) node into each cluster
	float makeAssignment(int ** assignments, float ** minRatios, float ***ratios);
	// function that implements new face index buffer
	void newClusterMean(int *piIndexBufferIn, int *piClustersIn,
		Vector ** pvFramesPatchesPositions, Vector * pvCameraPosiitons, int ** assignments, int clusterId, int *newMean);

};

bool compareClusterBuffer(int clusterId, int ** means, int *tempMean, int **assignments,
	MeGLWindow meWindow, float *pfCameraPositions, float **pfFramesVertexPositionsIn,
	int numFrames, int numViews, int numVertices, int numFaces);
void setFolderPath(char *objFolder, int characterId, int aniId);
void getBasicInformation(char * objFolder, int* numFaces, int* numVertices);
void loadData(int * piIndexBuffer, float ** pfFramesVertexPositionsIn, int vertexCount, int faceCount, char* objFolder, int duration);
void loadCameras(int characterId, int aniId, float *pfCameraPositions, int numViews);

#endif 

