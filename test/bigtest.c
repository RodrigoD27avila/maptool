#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map.h>

static MAP map;

void exit_test()
{
	MAP_Destroy(&map);
}

void print_and_exit(int error)
{
	fprintf(stderr, "%s\n", MAP_GetErrorString(error));
	exit(EXIT_FAILURE);
}

int main()
{
	int error = 0;
	atexit(exit_test);

	printf("INIT TEST0\n");
	if ((error = MAP_Init(&map, "test.map", "RODRIGOMAPFILE")) < 0) {
		print_and_exit(error);		
	}
	printf("TEST0 OK!\n");

	printf("INIT TEST1\n");
	if ((error = MAP_HeaderInit(&map, "LOL", "MAP TEST 01234567890011"
		"2233445566778899", "001", 1)) < 0) {
		print_and_exit(error);		
	}
	printf("TEST1 OK!\n");
	
	printf("INIT TEST2\n");
	if (( error = MAP_LayerInit(&map, 0, -8, 500, 500, 1)) < 0) {
		print_and_exit(error);		
	}
	printf("TEST2 OK!\n");

	printf("INIT TEST3\n");
	int i, j;
	for (i=0; i < 50; i++) {
		for (j=0; j < 50; j++) {

			if ((error = MAP_PushBack(&map, 0, i, j, &i)) < 0) {
				print_and_exit(error);		
			}
		}
	}
	printf("TEST3 OK!\n");

	printf("INIT TEST4\n");
	if ((error = MAP_Write(&map)) < 0) {
		print_and_exit(error);		
	}
	printf("TEST4 OK!\n");


	FILE *in = NULL;
	in = fopen("test.map", "rb");
	if (in == NULL) {
		print_and_exit(-MAP_ENULL);
	}

	printf("INIT TEST5\n");

	MAP_Header header;
	memset(&header, 0, sizeof(MAP_Header));

	error = MAP_GetHeaderInfo(header.id, header.name, header.version,
		&header.num_layers, in);

	if (error) {
		print_and_exit(error);
	}

	
	int result = -100;

	result = memcmp(map.header, &header, sizeof(MAP_Header));
	if (result != 0) {
		printf("FAILED!!\n");	
		print_and_exit(0);
	}

	printf("RESULT: %i\n", result);
	printf("TEST5 OK!\n");
	printf("INIT TEST6\n");
	MAP_Layer layer;
	memset(&layer, 0, sizeof(MAP_Layer));

	error = MAP_GetLayerInfo(0, &layer.id, &layer.size, &layer.offset,
		&layer.entry_bytes, &layer.width, &layer.height, in);

	if (error) {
		print_and_exit(error);
	}
	
	result = memcmp(map.layers[0], &layer, (4+8+4+2+2));
	if (result != 0) {
		printf("FAILED!!\n");	
		print_and_exit(0);
	}
	
	printf("RESULT: %i\n", result);
	printf("TEST6 OK!\n");
	printf("INIT TEST7\n");

	char *buffer = malloc(layer.size);
	error = MAP_GetLayerData(buffer, layer.size, layer.offset, in);
	
	if (error) {
		print_and_exit(error);
	}
	
	result = memcmp(map.layers[0]->map, buffer, layer.size);
	if (result != 0) {
		printf("FAILED!!\n");	
		print_and_exit(0);
	}
	free(buffer);

	printf("RESULT: %i\n", result);
	printf("TEST7 OK!\n");

	fclose(in);
	MAP_Destroy(&map);
	return 0;
}
