#include <stdio.h>
#include <string>
#include <stdlib.h>

#include "Obj_parser.h"
#include "List.h"
#include "string_extra.h"

#define WHITESPACE " \t\n\r"

void obj_free_half_list(list *listo)
{
	list_delete_all(listo);
	free(listo->names);
}

int obj_convert_to_list_index(int current_max, int index)
{
	if(index == 0)  //no index
		return -1;

	if(index < 0)  //relative to current list position
		return current_max + index;

	return index - 1;  //normal counting index
}

void obj_free_temp_storage(obj_growable_scene_data *growable_data)
{
	obj_free_half_list(&growable_data->vertex_list);
	obj_free_half_list(&growable_data->face_list);
}


void obj_convert_to_list_index_v(int current_max, int *indices)
{
	for(int i=0; i<MAX_VERTEX_COUNT; i++)
		indices[i] = obj_convert_to_list_index(current_max, indices[i]);
}

void obj_init_temp_storage(obj_growable_scene_data *growable_data)
{
	list_make(&growable_data->vertex_list, 10, 1);
	list_make(&growable_data->face_list, 10, 1);

}

obj_vector* obj_parse_vector()
{
	obj_vector *v = (obj_vector*)malloc(sizeof(obj_vector));
	v->e[0] = atof( strtok(NULL, WHITESPACE));
	v->e[1] = atof( strtok(NULL, WHITESPACE));
	v->e[2] = atof( strtok(NULL, WHITESPACE));
	return v;
}

int obj_parse_vertex_index(int *vertex_index)
{
	char *temp_str;
	char *token;
	int vertex_count = 0;

	while( (token = strtok(NULL, WHITESPACE)) != NULL)
	{
		vertex_index[vertex_count] = atoi( token );

		if(contains(token, "//"))  //normal only
		{
			temp_str = strchr(token, '/');
			temp_str++;
			//normal_index[vertex_count] = atoi( ++temp_str );
		}
		else if(contains(token, "/"))
		{
			temp_str = strchr(token, '/');
			//texture_index[vertex_count] = atoi( ++temp_str );

			if(contains(temp_str, "/"))
			{
				temp_str = strchr(temp_str, '/');
				//normal_index[vertex_count] = atoi( ++temp_str );
			}
		}

		vertex_count++;
	}

	return vertex_count;
}

obj_face* obj_parse_face(obj_growable_scene_data *scene)
{
	int vertex_count;
	obj_face *face = (obj_face*)malloc(sizeof(obj_face));

	vertex_count = obj_parse_vertex_index(face->vertex_index);
	obj_convert_to_list_index_v(scene->vertex_list.item_count,face->vertex_index);
	face->vertex_count = vertex_count;

	return face;
}

int obj_parse_obj_file(obj_growable_scene_data *growable_data,char *filename)
{
	FILE * obj_file_stream;
	char *current_token = NULL;
	char current_line[OBJ_LINE_SIZE];
	int line_number = 0;

	// open scene
	obj_file_stream = fopen( filename, "r");
	if(obj_file_stream == 0)
	{
		fprintf(stderr, "Error reading file: %s\n", filename);
		return 0;
	}
	//parser loop
	while( fgets(current_line, OBJ_LINE_SIZE, obj_file_stream) )
	{
		current_token = strtok(current_line, " \t\n\r");
		line_number++;

		//skip comments
		if( current_token == NULL || current_token[0] == '#')
			continue;

		//parse objects
		else if( strequal(current_token, "v") ) //process vertex
		{
			list_add_item(&growable_data->vertex_list,  obj_parse_vector(), NULL);
		}

		else if ( strequal(current_token, "f")) //process face
		{
			obj_face *face = obj_parse_face(growable_data); // ? waste memory?
			list_add_item(&growable_data->face_list, face, NULL);
		}
		else if( strequal(current_token, "vt") ) //texture name
		{ }
		else if( strequal(current_token, "o") ) //object name
		{ }
		else if( strequal(current_token, "s") ) //smoothing
		{ }
		else if( strequal(current_token, "g") ) // group
		{ }	

		else
		{
			printf("Unknown command '%s' in scene code at line %i: \"%s\".\n",
				current_token, line_number, current_line);
		}
	}
	fclose(obj_file_stream);

	return 1;
}

void obj_copy_to_out_storage(obj_scene_data *data_out, obj_growable_scene_data *growable_data)
{
	data_out->vertex_count = growable_data->vertex_list.item_count;

	data_out->face_count = growable_data->face_list.item_count;

	data_out->vertex_list = (obj_vector**)growable_data->vertex_list.items;

	data_out->face_list = (obj_face**)growable_data->face_list.items;
}

int parse_obj_scene(obj_scene_data *data_out, char *filename)
{
	obj_growable_scene_data growable_data;

	obj_init_temp_storage(&growable_data);
	if( obj_parse_obj_file(&growable_data, filename) == 0)
		return 0;

	obj_copy_to_out_storage(data_out, &growable_data);

}

void delete_obj_data(obj_scene_data *data_out)
{
	int i;

	for(i=0; i<data_out->vertex_count; i++)
		free(data_out->vertex_list[i]);
	free(data_out->vertex_list);

	for(i=0; i<data_out->face_count; i++)
		free(data_out->face_list[i]);
	free(data_out->face_list);
}