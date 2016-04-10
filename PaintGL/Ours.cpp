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

bool compareClusterBuffer(int clusterId, int ** means, int *tempMean, int **assignments,
	MeGLWindow meWindow, float *pfCameraPositions, float **pfFramesVertexPositionsIn,
	int numFrames, int numViews, int numVertices, int numFaces)
{
	// why we need swap buffer before reading atomic counters
	printf("\n ");
	printf("update cluster %u buffer \n", clusterId);
	bool clusterMove = false;
	GLuint oldDrawn = 0;
	meWindow.setZeroAtomicBuffer();
	oldDrawn = meWindow.readAtomicBuffer();
	for (int frameId = 0; frameId < numFrames; frameId++)
	{
		meWindow.setClusterCamera(pfCameraPositions, numViews, assignments, frameId, clusterId);
		meWindow.subLoadGeo(pfFramesVertexPositionsIn[frameId], numVertices, means[clusterId], numFaces);

		meWindow.render(numViews);
		meWindow.showGL();
	}
	oldDrawn = meWindow.readAtomicBuffer();
	printf("old drawn pixels: %u\n", oldDrawn);


	meWindow.setZeroAtomicBuffer();
	GLuint newDrawn = 0;
	for (int frameId = 0; frameId < numFrames; frameId++)
	{
		meWindow.subLoadGeo(pfFramesVertexPositionsIn[frameId], numVertices, tempMean, numFaces);
		meWindow.setClusterCamera(pfCameraPositions, numViews, assignments, frameId, clusterId);

		meWindow.render(numViews);
		meWindow.showGL();
	}
	newDrawn = meWindow.readAtomicBuffer();
	printf("new drawn pixels: %u\n", newDrawn);


	if (newDrawn < oldDrawn)
	{
		printf("replace old index buffer with new index buffer \n");
		memcpy(means[clusterId], tempMean, numFaces * 3 * sizeof(int));
		clusterMove = true;
	}
	else
	{
		printf("cluster index buffer remain the same \n");
	}
	return clusterMove;
}

void setFolderPath(char *objFolder, int characterId, int aniId)
{
	char Character[5][20] = { "Ganfaul_M_Aure", "Kachujin_G_Rosales", "Maw_J_Laygo", "Nightshade" };
	char Animation[6][40] = { "dancing_maraschino_step", "Standing_2H_Cast_Spell", "Standing_2H_Magic_Area_Attack", "Standing_Jump", "Standing_React_Death_Backward", "Standing_React_Large_From_Back" };

	strcpy(objFolder, "D:/TriangleOrdering/VF/Obj/");
	strcat(objFolder, Character[characterId]);
	strcat(objFolder, "/");
	strcat(objFolder, Animation[aniId]);
	strcat(objFolder, "/loop_subdiv1/");
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
	}

}

void loadCameras(int characterId, int aniId, float *pfCameraPositions, int numViews)
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
	for (int i = 0; i < numViews * 3; i++)
	{
		fscanf(myFile, "%f \n", &pfCameraPositions[i]);
	}
	fclose(myFile);
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

