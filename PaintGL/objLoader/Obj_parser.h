#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H

#include "List.h"

#define OBJ_FILENAME_LENGTH 500
#define OBJ_LINE_SIZE 5000
#define MAX_VERTEX_COUNT 3 //can only handle triangles

typedef struct obj_face
{
	int vertex_index[MAX_VERTEX_COUNT];
	//int vertex_count;
};

typedef struct obj_vector
{
	double v[3];
};

typedef struct obj_growable_scene_data
{
	//	vector extreme_dimensions[2];
	char scene_filename[OBJ_FILENAME_LENGTH];
	
	list vertex_list;
	
	list face_list;
};

typedef struct obj_scene_data
{
	obj_vector **vertex_list;
	obj_face **face_list;

	int vertex_count;
	int face_count;
};

int parse_obj_scene(obj_scene_data *data_out, char *filename);
void delete_obj_data(obj_scene_data *data_out);

#endif