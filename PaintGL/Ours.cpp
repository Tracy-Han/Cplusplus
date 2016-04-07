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
	_numClusters = Clusters;
	_pickIds = new int[_numClusters];
	memcpy(_pickIds, viewIds, _numClusters*sizeof(int));
	/*printf("pickIds: %u, %u, %u, %u, %u\n", _pickIds[0], _pickIds[1], _pickIds[2], _pickIds[3], _pickIds[4]);
	printf("# of Clusters: %u\n", _numClusters);*/

	_numFrames = framesNum;
	_numFaces = facesNum;
	_numVertices = verticesNum;
	_numPatches = patchesNum;
	_numViews = viewsNum;

	//printf("# of Frames: %u\n", _numFrames);
	//printf("# of Faces: %u\n", _numFaces);
	//printf("# of vertices: %u\n", _numVertices);
	//printf("# of Patches: %u\n", _numPatches);

}
//function that implements getting patches positions
void ours::computePatchPos(int *piIndexBufferIn,
	float *pfVertexPositionsIn,
	int *piPatchesIn,
	Vector * pvPatchesPositions)
{
	int *p = piIndexBufferIn;
	Vector *pvVertexPositionsIn = (Vector *)pfVertexPositionsIn;

	Vector *pvPatchPositions = new Vector[_numPatches];
	memset(pvPatchPositions, 0.0, _numPatches*sizeof(Vector));
	Vector *pvPatchNormals = new Vector[_numPatches];
	memset(pvPatchNormals, 0.0, _numPatches*sizeof(Vector));
	float *pfPatchAreas = new float[_numPatches];
	memset(pfPatchAreas, 0.0, _numPatches*sizeof(float));
	Vector vMeshPositions = Vector(0, 0, 0);

	/*for (int i = 0; i < _numPatches; i++)
	{
		pvPatchPositions[i] = Vector(0, 0, 0);
		pvPatchNormals[i] = Vector(0, 0, 0);
		pfPatchAreas[i] = 0.0f;
	}*/

	/* compute patch positions */
	int c = 0, cstart = 0; int i, j;
	int cnext = piPatchesIn[1];
	float fMArea = 0.f; float fCArea = 0.f;
	for (i = 0; i <= _numFaces; i++)
	{
		if (i == cnext)
		{
			pfPatchAreas[c] = fCArea;
			pvPatchPositions[c] /= fCArea * 3.f;
			pvPatchNormals[c].normalize();
			c++;
			if (c == _numPatches)
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
	//printf("# of patches: %u \n", _numPatches);
	for (int i = 0; i < _numVertices; i++)
	{
		pvVertexPositionsIn[i] -= vMeshPositions;
	}
	for (int i = 0; i < _numPatches; i++){
		pvPatchesPositions[i] = Vector(pvPatchPositions[i].v[0], pvPatchPositions[i].v[1], pvPatchPositions[i].v[2]) - vMeshPositions;
	}

	delete[] pfPatchAreas;
	delete[] pvPatchNormals;
	delete[] pvPatchPositions;
}

void ours::depthSortPatch(Vector viewpoint, Vector *pvAvgPatchesPositions, int *piIndexBufferIn, int *piPachesIn, int * piIndexBufferTmp)
{
	int i, j;
	patchSort *viewToPatch = new patchSort[_numPatches];
	memset(viewToPatch, 0, _numPatches*sizeof(patchSort));
	//printf("averagePatchPosition 1 %f , %f , %f\n", pvAvgPatchesPositions[0].v[0], pvAvgPatchesPositions[0].v[1], pvAvgPatchesPositions[0].v[2]);
	//printf("averagePatchPosition 100 %f , %f , %f\n", pvAvgPatchesPositions[99].v[0], pvAvgPatchesPositions[99].v[1], pvAvgPatchesPositions[99].v[2]);
	for (i = 0; i < _numPatches; i++)
	{
		viewToPatch[i].id = i;
		viewToPatch[i].dist = dist(viewpoint, pvAvgPatchesPositions[i]);
	}
	std::sort(viewToPatch, viewToPatch + _numPatches, sortPatch);
	//printf("viewToPatch 1 dist %f\n", viewToPatch[0].dist);
	//printf("viewToPatch 10 dist %f\n", viewToPatch[9].dist);
	//printf("viewToPatch %u dist %f\n",_numPatches, viewToPatch[_numPatches-1].dist);

	int jj = 0;
	for (i = 0; i < _numPatches; i++)
	{
		for (j = piPachesIn[viewToPatch[i].id] * 3; j < piPachesIn[viewToPatch[i].id + 1] * 3; j++)
		{
			piIndexBufferTmp[jj++] = piIndexBufferIn[j];
		}
	}
	
	delete[] viewToPatch;
}

void ours::initMeans(int ** means, Vector ** pvFramesPatchesPositions, int * piIndexBufferIn, int * piPachesIn, float * pfCameraPositions)
{
	int i, j;

	Vector *pvAvgPatchesPositions = new Vector[_numPatches];
	memset(pvAvgPatchesPositions, 0, _numPatches *sizeof(Vector));

	for (i = 0; i < _numFrames; i++)
	{
		for (j = 0; j < _numPatches; j++)
		{
			pvAvgPatchesPositions[j] += pvFramesPatchesPositions[i][j];
		}
	}
	for (j = 0; j < _numPatches; j++)
	{
		pvAvgPatchesPositions[j] /= _numFrames;
	}

	for (i = 0; i < _numClusters; i++)
	{
		Vector viewpoint = Vector(pfCameraPositions[_pickIds[i] * 3], pfCameraPositions[_pickIds[i] * 3 + 1], pfCameraPositions[_pickIds[i] * 3 + 2]);
		depthSortPatch(viewpoint, pvAvgPatchesPositions, piIndexBufferIn, piPachesIn, means[i]);
	}
	delete[] pvAvgPatchesPositions;
}

void ours::newClusterMean(int *piIndexBufferIn, int *piClustersIn,
	Vector ** pvFramesPatchesPositions, Vector * pvCameraPosiitons, int ** assignments, int clusterId, int *newMean)
{

	clusterAssign *cluster = new clusterAssign[_numFrames*_numViews];
	memset(cluster, 0, _numFrames*_numViews*sizeof(clusterAssign));
	patchSort *viewToPatch = new patchSort[_numPatches];
	memset(viewToPatch, 0, _numPatches*sizeof(patchSort));

	int i, j;
	/* what node is in cluster return avgRatio */
	int count = 0;
	for (i = 0; i < _numFrames; i++)
	{
		for (j = 0; j < _numViews; j++)
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
	for (i = 0; i < _numPatches; i++)
	{
		viewToPatch[i].id = i;
	}
	for (i = 0; i < count; i++)
	{
		frameId = cluster[i].frameId;
		viewId = cluster[i].viewId;
		for (j = 0; j < _numPatches; j++)
		{
			viewToPatch[j].dist += dist(pvCameraPosiitons[viewId], pvFramesPatchesPositions[frameId][j]);
		}
	}
	std::sort(viewToPatch, viewToPatch + _numPatches, sortPatch);

	int jj = 0;
	for (i = 0; i < _numPatches; i++)
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
float ours::makeAssignment(int ** assignments, float ** minRatios, float ***ratios)
{
	int x, y, z;
	float averageRatio = 0;
	patchSort *tempRatio = new patchSort[_numClusters];
	memset(tempRatio, 0, _numClusters*sizeof(patchSort));
	for (x = 0; x < _numFrames; x++)
	{
		for (z = 0; z < _numViews; z++)
		{
			for (int y = 0; y < _numClusters; y++)
			{
				tempRatio[y].id = y;
				tempRatio[y].dist = ratios[x][y][z];
			}

			std::sort(tempRatio, tempRatio + _numClusters, sortPatch);
			minRatios[x][z] = tempRatio[0].dist;
			assignments[x][z] = tempRatio[0].id;
			averageRatio += minRatios[x][z];
		}
	}
	averageRatio /= _numFrames*_numViews;
	return averageRatio;

}