/*See Copyright Notice in map.h*/

#ifndef MAPTOOL_H_INCLUDED
#define MAPTOOL_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAP_MAX_LAYERS   1024

#define MAP_MAGIC_SIZE   16
#define MAP_ID_SIZE      6
#define MAP_NAME_SIZE    64
#define MAP_VERSION_SIZE 4

typedef struct {
	int      id;
	int	 size;
	long int offset;
	
	int       entry_bytes;
	short int width;
	short int height;

	char *map;
} MAP_Layer;

typedef struct {
	char id[MAP_ID_SIZE];
	char name[MAP_NAME_SIZE];
	char version[MAP_VERSION_SIZE];
	int  num_layers;
} MAP_Header;

typedef struct {
	char *name;
	char magic[MAP_MAGIC_SIZE];

	MAP_Header *header;
	MAP_Layer  *layers[MAP_MAX_LAYERS];
} MAP;


int MAP_Init(MAP *map, char *filename, char *magic);
int MAP_HeaderInit(MAP *map, char *id, char *name, char *version,
	int num_layers);
int MAP_LayerInit(MAP *map, unsigned int layer, int id, short int width,
	short int height, int entry_bytes);

int MAP_Destroy(MAP *map);

int MAP_PushBack(MAP *map, unsigned int layer, unsigned int x,
	unsigned int y, void *entry);

int MAP_Write(MAP *map);

int MAP_GetHeaderInfo(char *id, char *name, char *version,
	int *num_layers, FILE *map);
int MAP_GetHeaderInfoMem(char *id, char *name, char *version,
	int *num_layers, char *buffer);

int MAP_GetLayerInfo(int layer, int *id, int *size, long int *offset,
	int *entry_bytes, short int *width, short int *height, FILE *map);
int MAP_GetLayerInfoMem(int layer, int *id, int *size, long int *offset,
	int *entry_bytes, short int *width, short int *height, char *buffer);

int MAP_GetLayerData(char *mapbuffer, int len, long int pos,
	FILE *map);
int MAP_GetLayerDataMem(char *mapbuffer, int len, long int pos,
	char *buffer);

#ifdef __cplusplus
}
#endif

#endif /*MAPTOOL_H_INCLUDED*/
