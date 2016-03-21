#include <stdio.h>
#include "ObjLoader.h"

void printVector(obj_vector *v)
{
	printf("%.2f,", v->e[0] );
	printf("%.2f,", v->e[1] );
	printf("%.2f  ", v->e[2] );
}

int main(int argc, char **argv)
{
	objLoader *objData = new objLoader();
	objData->load("frame1.obj");
	
	printf("Number of Vertex: %i \n",objData->vertexCount);

	printf("Number of faces: %i\n", objData->faceCount);
//	for(int i=0; i<objData->faceCount; i++)
	for(int i=0; i<10; i++)
	{
		obj_face *o = objData->faceList[i];
		printf(" face ");
//		printf( "Number of vertex: %i\n",o->vertex_count);
		for(int j=0; j<3; j++)
		{
			printVector(objData->vertexList[ o->vertex_index[j] ]);
		}
		printf("\n");
	}
	objData->delete_objLoader();
	printf("\n");

	getchar();
}