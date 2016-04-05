#include "Ours.h"

class patchSort
{
public:
	float dist;// distance
	int id;//index
};
class clusterAssign
{
public:
	int frameId;
	int viewId;
};
bool sortPatch(const patchSort &a, const patchSort &b)
{
	return a.dist < b.dist;
}

void ours::setParameter(int *viewIds, int Clusters, int framesNum, int facesNum, int verticesNum, int patchesNum, int viewsNum)
{
	numClusters = Clusters;
	pickIds = new int[numClusters];
	memcpy(pickIds, viewIds, numClusters*sizeof(int));
	printf("pickIds: %u, %u, %u, %u, %u\n", pickIds[0], pickIds[1], pickIds[2], pickIds[3], pickIds[4]);
	printf("# of Clusters: %u\n", numClusters);

	numFrames = framesNum;
	numFaces = facesNum;
	numVertices = verticesNum;
	numPatches = patchesNum;
	numViews = viewsNum;

	//printf("# of Frames: %u\n", numFrames);
	//printf("# of Faces: %u\n", numFaces);
	//printf("# of vertices: %u\n", numVertices);
	//printf("# of Patches: %u\n", numPatches);

}
//function that implements getting patches positions
void ours::computePatchPos(int *piIndexBufferIn,
						   float *pfVertexPositionsIn,
						   int *piPatchesIn,
						   Vector * pvPatchesPositions)
{
	int *p = piIndexBufferIn;
	Vector *pvVertexPositionsIn = (Vector *)pfVertexPositionsIn;

	Vector *pvPatchPositions = new Vector[numPatches];
	Vector *pvPatchNormals = new Vector[numPatches];
	float *pfPatchAreas = new float[numPatches];
	Vector vMeshPositions = Vector(0, 0, 0);
	for (int i = 0; i < numPatches; i++)
	{
		pvPatchPositions[i] = Vector(0, 0, 0);
		pvPatchNormals[i] = Vector(0, 0, 0);
		pfPatchAreas[i] = 0.0f;
	}

	/* compute patch positions */
	int c = 0, cstart = 0; int i,j;
	int cnext = piPatchesIn[1];
	float fMArea = 0.f; float fCArea = 0.f;
	for (i = 0; i <= numFaces; i++)
	{
		if (i == cnext)
		{
			pfPatchAreas[c] = fCArea;
			pvPatchPositions[c] /= fCArea * 3.f;
			pvPatchNormals[c].normalize();
			c++;
			if (c == numPatches)
				break;
			cstart = i;
			cnext = piPatchesIn[c + 1];
			fCArea = 0.f;
		}
		Vector vNormal = cross(pvVertexPositionsIn[p[2]] - pvVertexPositionsIn[p[0]],
			pvVertexPositionsIn[p[1]] - pvVertexPositionsIn[p[0]]);
		float fArea = vNormal.length();
		if (fArea > 0.f)
		{
			vNormal /= fArea;
		}
		else
		{
			fArea = 0.f;
			vNormal = Vector(0, 0, 0);
		}
		for (j = 0; j < 3; j++)
		{
			Vector *vp = (Vector *)&pfVertexPositionsIn[(*p) * 3];
			vMeshPositions += *vp * fArea;
			pvPatchPositions[c] += *vp * fArea;
			p++;
		}
		pvPatchNormals[c] += vNormal;
		fMArea += fArea;
		fCArea += fArea;
	}
	/* I might could move the object by this mesh positions */
	vMeshPositions /= fMArea * 3.f;
	//printf("mesh center position: %f ,%f ,%f \n",vMeshPositions.v[0],vMeshPositions.v[1],vMeshPositions.v[2]);
	/* what about all delete by mesh positions */
	//printf("# of patches: %u \n", numPatches);
	for (int i = 0; i < numVertices; i++)
	{
		pvVertexPositionsIn[i] -= vMeshPositions;
	}
	for (int i = 0; i < numPatches; i++){
		pvPatchesPositions[i] = Vector(pvPatchPositions[i].v[0], pvPatchPositions[i].v[1], pvPatchPositions[i].v[2]) - vMeshPositions;
	}

	delete[] pfPatchAreas;
	delete[] pvPatchNormals;
	delete[] pvPatchPositions;
}

void ours::depthSortPatch(Vector viewpoint, Vector *pvAvgPatchesPositions, int *piIndexBufferIn, int *piPachesIn, int * piIndexBufferTmp)
{
	int i, j;
	patchSort *viewToPatch = new patchSort[numPatches];

	for (i = 0; i < numPatches; i++)
	{
		viewToPatch[i].id = i;
		viewToPatch[i].dist = dist(viewpoint, pvAvgPatchesPositions[i]);
	}
	std::sort(viewToPatch, viewToPatch + numPatches, sortPatch);

	int jj = 0;
	for (i = 0; i < numPatches; i++)
	{
		for (j = piPachesIn[viewToPatch[i].id] * 3; j < piPachesIn[viewToPatch[i].id + 1] * 3; j++)
		{
			piIndexBufferTmp[jj++] = piIndexBufferIn[j];
		}
	}

	delete[] viewToPatch;
}

void ours::initMeans(int ** means, Vector ** pvFramesPatchesPositions, int * piIndexBufferIn, int * piPachesIn,   float * pfCameraPositions)
{
	int i, j;
	
	Vector *pvAvgPatchesPositions = new Vector[numPatches];

	for (i = 0; i < numFrames; i++)
	{
		for (j = 0; j < numPatches; j++)
		{
			pvAvgPatchesPositions[j] += pvFramesPatchesPositions[i][j];
		}
	}
	for (j = 0; j < numPatches; j++)
	{
		pvAvgPatchesPositions[j] /= numFrames;
	}

	for (i = 0; i < numClusters; i++)
	{
		Vector viewpoint = Vector(pfCameraPositions[pickIds[i] * 3], pfCameraPositions[pickIds[i] * 3 + 1], pfCameraPositions[pickIds[i] * 3 + 2]);
		depthSortPatch(viewpoint, pvAvgPatchesPositions, piIndexBufferIn, piPachesIn, means[i]);
	}
	delete[] pvAvgPatchesPositions;
}

void ours::newClusterMean(int *piIndexBufferIn, int *piClustersIn,
	Vector ** pvFramesPatchesPositions, Vector * pvCameraPosiitons, int ** assignments, int clusterId,int *newMean)
{

	clusterAssign *cluster = new clusterAssign[numFrames*numViews];
	patchSort *viewToPatch = new patchSort[numPatches];

	int i, j;
	/* what node is in cluster return avgRatio */
	int count = 0; 
	for (i = 0; i < numFrames; i++)
	{
		for (j = 0; j < numViews; j++)
		{
			if (assignments[i][j] == clusterId)
			{
				cluster[count].frameId = i;
				cluster[count].viewId = j;
				count++;
			}
		}
	}

	/* compute new index buffer */
	int frameId, viewId;
	for (i = 0; i < numPatches; i++)
	{
		viewToPatch[i].id = i;
	}
	for (i = 0; i < count; i++)
	{
		frameId = cluster[i].frameId;
		viewId = cluster[i].viewId;
		for (j = 0; j < numPatches; j++)
		{
			viewToPatch[j].dist += dist(pvCameraPosiitons[viewId], pvFramesPatchesPositions[frameId][j]);
		}
	}
	std::sort(viewToPatch, viewToPatch + numPatches, sortPatch);

	int jj = 0;
	for (i = 0; i < numPatches; i++)
	{
		for (j = piClustersIn[viewToPatch[i].id] * 3; j < piClustersIn[viewToPatch[i].id + 1] * 3; j++)
		{
			newMean[jj++] = piIndexBufferIn[j];
		}
	}

	delete[] cluster;
	delete[] viewToPatch;

}

// function that implements the assignments
float ours::makeAssignment(int ** assignments, float ** minRatios, float ratios[75][5][162])
{
	int x, y, z;
	float averageRatio = 0;
	patchSort *tempRatio = new patchSort[numClusters];
	for (x = 0; x < numFrames; x++)
	{
		for (z = 0; z < numViews; z++)
		{
			for (int y = 0; y < numClusters; y++)
			{
				tempRatio[y].id = y;
				tempRatio[y].dist = ratios[x][y][z];
			}

			std::sort(tempRatio, tempRatio + numClusters, sortPatch);
			minRatios[x][z] = tempRatio[0].dist;
			assignments[x][z] = tempRatio[0].id;
			averageRatio += minRatios[x][z];
		}
	}
	averageRatio /= numFrames*numViews;
	return averageRatio;

}