/*See Copyright Notice in map.h*/

#include "maptool.h"
#include "maperror.h"

const int MAP_TABLE_ENTRY_BYTES = (4+4+8+4+2+2);

int MAP_Init(MAP *map, char *filename, char *magic)
{
	int error = 0;

	memset(map, 0, sizeof(MAP));

	map->name = filename;
	strncpy(map->magic, magic, MAP_MAGIC_SIZE-1);
	
	map->header = malloc(sizeof(MAP_Header));
	if (map->header == NULL) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		error = -MAP_EALLOC;
		goto end_with_error;
	}
	memset(map->header, 0, sizeof(MAP_Header));

	return 0;

end_with_error:
	MAP_Destroy(map);
	return error;
}

int MAP_HeaderInit(MAP *map, char *id, char *name, char *version,
	int num_layers)
{
	int error = 0;

	if (num_layers >= MAP_MAX_LAYERS) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		error = -MAP_ELMAX;
		goto end_with_error;		
	}

	MAP_Header *header = map->header;

	strncpy(header->id     , id     , MAP_ID_SIZE-1);
	strncpy(header->name   , name   , MAP_NAME_SIZE-1);
	strncpy(header->version, version, MAP_VERSION_SIZE-1);

	header->num_layers  = num_layers;
	return 0;

end_with_error:
	MAP_Destroy(map);
	return error;
}

int MAP_LayerInit(MAP *map, unsigned int layer, int id, short int width,
        short int height, int entry_bytes)
{
	int error = 0;

	MAP_Layer *layerdata = malloc(sizeof(MAP_Layer));
	if (layerdata == NULL) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		error = -MAP_EALLOC;
		goto end_with_error;
	}

	layerdata->id          = id;
	layerdata->entry_bytes = entry_bytes;
	layerdata->width       = width;
	layerdata->height      = height;
	layerdata->size        = (width * height * entry_bytes);

	char *mapdata = malloc(layerdata->size);
	if (mapdata == NULL) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		error = -MAP_EALLOC;
		goto end_with_error;
	}
	layerdata->map = mapdata;

	map->layers[layer] = layerdata;

	return 0;

end_with_error:
	MAP_Destroy(map);
	return error;
}

int MAP_Destroy(MAP *map)
{
	if (map->header) {
		int i;
		for (i=0; i < map->header->num_layers; i++) {
			MAP_Layer *layer = map->layers[i];

			free(layer->map);
			free(layer);
		}
		free(map->header);
	}

	memset(map, 0, sizeof(MAP));
	return 0;
}

int MAP_PushBack(MAP *map, unsigned int layer, unsigned int x,
	unsigned int y, void *entry)
{
	int error  = 0;

	if (layer >= map->header->num_layers) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		error = -MAP_EINDEX;
		goto end_with_error;	
	}

	int width  = map->layers[layer]->width;
	int height = map->layers[layer]->height;
	int bytes  = map->layers[layer]->entry_bytes; 

	if ((x >= width) || (y >= height)) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		error = -MAP_EINDEX;
		goto end_with_error;	
	}

	MAP_Layer *layerdata = map->layers[layer];

	int pos = ((width * y) + x) * bytes;
	memcpy(layerdata->map + pos, entry, bytes);

	return 0;

end_with_error:
	MAP_Destroy(map);
	return error;
}

static int map_write_layers(MAP *map, FILE *out)
{
	int i;
	for (i=0; i < map->header->num_layers; i++) {
		MAP_Layer *layer = map->layers[i];

		if ((fwrite(layer->map, 1, layer->size, out)) <= 0) {
			#ifdef MAP_DEBUG
				printf("%s: %i\n", __FILE__, __LINE__);
			#endif
			return -MAP_EWRITE;
		}
	}	
}

static int map_create_table(MAP *map, int bytes, char **buffer, long pos)
{
	char *newbuffer = malloc(bytes);
	if (newbuffer == NULL) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		return -MAP_EALLOC;
	}
	memset(newbuffer, 0, bytes);

	long file_pos = pos + bytes;

	int i;
	for (i=0; i<map->header->num_layers; i++) {
		int p = (i * MAP_TABLE_ENTRY_BYTES);

		MAP_Layer *layer = map->layers[i];
		layer->offset = file_pos;
		memcpy(p+newbuffer, layer, MAP_TABLE_ENTRY_BYTES);

		file_pos += layer->size;
	}

	*buffer = newbuffer;
	return 0;	
}

int MAP_Write(MAP *map)
{
	int error = 0;
	FILE *out = NULL;
	long bb = 0;

	out = fopen(map->name, "wb+");
	if (out == NULL) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		error = -MAP_EFILE;
		goto end_with_error;
	}

	/*write magic*/
	if ((bb += fwrite(map->magic, 1, MAP_MAGIC_SIZE, out)) <= 0) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		error = -MAP_EWRITE;
		goto end_with_error;
	}

	/*write header*/
	if ((bb += fwrite(map->header, 1, sizeof(MAP_Header), out)) <= 0) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		error = -MAP_EWRITE;
		goto end_with_error;
	}

	/*create table*/
	char *table_buffer = NULL;
	int   bytes  = (map->header->num_layers * MAP_TABLE_ENTRY_BYTES);

	if ((error = map_create_table(map, bytes,
			&table_buffer, ftell(out))) < 0) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		goto end_with_error;
	}

	/*write table*/
	if ((bb += fwrite(table_buffer, 1, bytes, out)) <= 0) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		error = -MAP_EWRITE;
		goto end_with_error;
	}

	/*write layers*/
	if ((error = map_write_layers(map, out)) < 0) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		goto end_with_error;
	}
	
	fflush(out);
	fclose(out);
	free(table_buffer);

	return 0;

end_with_error:
	if (out)          fclose(out);
	if (table_buffer) free(table_buffer);
	MAP_Destroy(map);
	return error;
}

static int map_read_header_from_file(MAP_Header *header, FILE *map)
{
	if ((fseek(map, MAP_MAGIC_SIZE, SEEK_SET)) < 0) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		return -MAP_ESEEK;
	}

	if ((fread(header, 1, sizeof(MAP_Header), map)) <= 0) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		return -MAP_EREAD;
	}

	return 0;
}

static int map_copy_header_info(MAP_Header *header, char *id, char *name,
	char *version, int *num_layers)
{
	if (id)      memcpy(id     , header->id     , MAP_ID_SIZE);
	if (name)    memcpy(name   , header->name   , MAP_NAME_SIZE);
	if (version) memcpy(version, header->version, MAP_VERSION_SIZE);

	if (num_layers)  *num_layers  = header->num_layers;

	return 0;
}

int MAP_GetHeaderInfo(char *id, char *name, char *version, int *num_layers,
	FILE *map)
{
	int error = 0;

	MAP_Header header = {0};
	if ((error = map_read_header_from_file(&header, map)) < 0) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		return error;
	}
	map_copy_header_info(&header, id, name, version, num_layers);
	
	return 0;
}

int MAP_GetHeaderInfoMem(char *id, char *name, char *version, int *num_layers,
	char *buffer)
{
	MAP_Header *header = NULL;
	header = (MAP_Header *)(buffer+MAP_MAGIC_SIZE);
	map_copy_header_info(header, id, name, version, num_layers);

	return 0;
}

static int map_copy_layer_info(MAP_Layer *layer, int *id, int *size,
	long int *offset, int *entry_bytes, short int *width,
	short int *height)
{
	if (id)           *id          = layer->id;
	if (size)         *size        = layer->size;
	if (offset)       *offset      = layer->offset;
	if (entry_bytes)  *entry_bytes = layer->entry_bytes;
	if (width)        *width       = layer->width;
	if (height)       *height      = layer->height;

	return 0;

}

static int map_read_layer_info_from_file(MAP_Layer *layer, int layer_num,
	FILE *map)
{
	int pos   = MAP_MAGIC_SIZE + sizeof(MAP_Header)
		+ (MAP_TABLE_ENTRY_BYTES * layer_num);

	if (fseek(map, pos, SEEK_SET) < 0) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		return -MAP_ESEEK;
	} 

	if (fread(layer, 1, MAP_TABLE_ENTRY_BYTES, map) <= 0) {
		#ifdef MAP_DEBUG
			printf("%s: %i\n", __FILE__, __LINE__);
		#endif
		return -MAP_EREAD;
	}
}

int MAP_GetLayerInfo(int layer, int *id, int *size, long int *offset,
        int *entry_bytes, short int *width, short int *height, FILE *map)
{
	int error = 0;
	MAP_Layer layerdata = {0};

	rewind(map);

	if ((error = map_read_layer_info_from_file(&layerdata,
		layer, map)) < 0) {
		return error;
	}
	map_copy_layer_info(&layerdata, id, size, offset, entry_bytes,
		width, height);

	return 0;
		
}

int MAP_GetLayerInfoMem(int layer, int *id, int *size, long int *offset,
        int *entry_bytes, short int *width, short int *height, char *buffer)
{
	MAP_Layer *layerdata = NULL;

	int pos   = MAP_MAGIC_SIZE + sizeof(MAP_Header)
		+ (MAP_TABLE_ENTRY_BYTES * layer);

	layerdata = (MAP_Layer *)(pos+buffer);
	map_copy_layer_info(layerdata, id, size, offset, entry_bytes,
		width, height);
}

int MAP_GetLayerData(char *mapbuffer, int len, long int pos,
        FILE *map)
{
	rewind(map);

	if ((fseek(map, pos, SEEK_SET)) < 0) {
		return -MAP_ESEEK;
	}

	if ((fread(mapbuffer, 1, len, map)) <= 0) {
		return -MAP_EREAD;
	}

	return 0;
}

int MAP_GetLayerDataMem(char *mapbuffer, int len, long int pos,
        char *buffer)
{
	memcpy(mapbuffer, buffer+pos, len);
	return 0;
}
