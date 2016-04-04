#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include "obj_parser.h"

class objLoader
{
public:
	objLoader() {}
	~objLoader()
	{
		delete_obj_data(&data);
	}

	int load(char *filename);
	void delete_objLoader();

	obj_vector **vertexList;	
	obj_face **faceList;
	
	int vertexCount;
	int faceCount;
	
private:
	obj_scene_data data;
};

#endif
