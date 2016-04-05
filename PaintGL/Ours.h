#ifndef OURS_H
#define OURS_H

#include <iostream>
#include <algorithm>

#include "vector.h"

class ours
{
private:
	int *pickIds;
	int numClusters;
	// get from preprocessing
	int numFrames;
	int numFaces;
	int numVertices;
	int numPatches;
	int numViews;

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

#endif 

