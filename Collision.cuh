#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "Ball.h"

#define RADIX_LENGTH 8
#define HOME_CELL 0x00
#define HOME_OBJECT 0x01
#define PHANTOM_CELL 0x01
#define PHANTOM_OBJECT 0x00


void collisionDetection(Ball *balls, float refreshInterval, float length, float width, float height, 
	float gridSize, int grid_x, int grid_y, int grid_z, int n);

void spatialSubdivision(Ball* balls, float length, float width, float height,
	float gridSize, int grid_x, int grid_y, int grid_z, int n, unsigned int num_blocks, unsigned int threads_per_block);

void radixSort(uint32_t* cells, uint32_t* objects, uint32_t* cells_temp, uint32_t* objects_temp,
	uint32_t* radix_sums, int n, uint32_t* indices, uint32_t* num_indices, unsigned int num_blocks, 
	unsigned int threads_per_block);