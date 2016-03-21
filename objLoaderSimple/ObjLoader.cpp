#include "objLoader.h"
#include "obj_parser.h"


int objLoader::load(char *filename)
{
	int no_error = 1;
	no_error = parse_obj_scene(&data, filename);
	if(no_error)
	{
		this->vertexCount = data.vertex_count;
		
		this->faceCount = data.face_count;
		
		this->vertexList = data.vertex_list;
		
		this->faceList = data.face_list;
	}

	return no_error;
}
void objLoader::delete_objLoader()
{
	delete_obj_data(&data);
}