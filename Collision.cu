#include <math.h>
#include <stdint.h>
#include "Collision.cuh"
#include "Ball.h"

__device__ float dist_(float x, float y, float z)
{
	return sqrt(x * x + y * y + z * z);
}


// 精确碰撞检测
__device__ bool isCollision(Ball& a, Ball& b)
{
	float dist_x = a.pos.x - b.pos.x;
	float dist_y = a.pos.y - b.pos.y;
	float dist_z = a.pos.z - b.pos.z;
	float dist = dist_(dist_x, dist_y, dist_z);
	if (dist < a.radius + b.radius)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// 碰撞后速度更新
__device__ void updateSpeed(Ball& a, Ball& b)
{
	float dist = 0;
	float dist_x = b.pos.x - a.pos.x;
	float dist_y = b.pos.y - a.pos.y;
	float dist_z = b.pos.z - a.pos.z;
	dist = dist_(dist_x, dist_y, dist_z);

	// 碰撞方向的速度
	float rate_collide_a = (a.speed.x * dist_x + a.speed.y * dist_y + a.speed.z * dist_z) / dist / dist;
	float normal_velocity_a_x = dist_x * rate_collide_a;
	float normal_velocity_a_y = dist_y * rate_collide_a;
	float normal_velocity_a_z = dist_z * rate_collide_a;

	float rate_collide_b = (b.speed.x * dist_x + b.speed.y * dist_y + b.speed.z * dist_z) / dist / dist;
	float normal_velocity_b_x = dist_x * rate_collide_b;
	float normal_velocity_b_y = dist_y * rate_collide_b;
	float normal_velocity_b_z = dist_z * rate_collide_b;

	// 垂直方向的速度
	float tangential_velocity_a_x = a.speed.x - normal_velocity_a_x;
	float tangential_velocity_a_y = a.speed.y - normal_velocity_a_y;
	float tangential_velocity_a_z = a.speed.z - normal_velocity_a_z;

	float tangential_velocity_b_x = b.speed.x - normal_velocity_b_x;
	float tangential_velocity_b_y = b.speed.y - normal_velocity_b_y;
	float tangential_velocity_b_z = b.speed.z - normal_velocity_b_z;

	// 更新速度，需考虑弹性系数和质量
	float normal_velocity_new_a_x = ((1 - a.coefficient) * (normal_velocity_a_x * (a.weight - b.weight) + normal_velocity_b_x * (2 * b.weight))) / (a.weight + b.weight);
	float normal_velocity_new_a_y = ((1 - a.coefficient) * (normal_velocity_a_y * (a.weight - b.weight) + normal_velocity_b_y * (2 * b.weight))) / (a.weight + b.weight);
	float normal_velocity_new_a_z = ((1 - a.coefficient) * (normal_velocity_a_z * (a.weight - b.weight) + normal_velocity_b_z * (2 * b.weight))) / (a.weight + b.weight);

	float normal_velocity_new_b_x = ((1 - b.coefficient) * (normal_velocity_a_x * (2 * a.weight) + normal_velocity_b_x * (b.weight - a.weight))) / (a.weight + b.weight);
	float normal_velocity_new_b_y = ((1 - b.coefficient) * (normal_velocity_a_y * (2 * a.weight) + normal_velocity_b_y * (b.weight - a.weight))) / (a.weight + b.weight);
	float normal_velocity_new_b_z = ((1 - b.coefficient) * (normal_velocity_a_z * (2 * a.weight) + normal_velocity_b_z * (b.weight - a.weight))) / (a.weight + b.weight);


	a.speed.x = normal_velocity_new_a_x + tangential_velocity_a_x;
	a.speed.y = normal_velocity_new_a_y + tangential_velocity_a_y;
	a.speed.z = normal_velocity_new_a_z + tangential_velocity_a_z;

	b.speed.x = normal_velocity_new_b_x + tangential_velocity_b_x;
	b.speed.y = normal_velocity_new_b_y + tangential_velocity_b_y;
	b.speed.z = normal_velocity_new_b_z + tangential_velocity_b_z;
}


__global__ void updateBalls(Ball* balls, float interval, float length, float width, float height, int n)
{
	int index = threadIdx.x + blockIdx.x * blockDim.x;
	int stride = blockDim.x * gridDim.x;
	for (int i = index; i < n; i += stride)
	{
		// s_t = s_0 + v * t
		balls[i].pos.x = balls[i].pos.x + balls[i].speed.x * interval;
		balls[i].pos.y = balls[i].pos.y + balls[i].speed.y * interval;
		balls[i].pos.z = balls[i].pos.z + balls[i].speed.z * interval;

		// 考虑撞墙
		if (balls[i].pos.x - balls[i].radius < -length)
		{
			balls[i].pos.x = -length + balls[i].radius;
			balls[i].speed.x = -balls[i].speed.x;
		}
		else if (balls[i].pos.x + balls[i].radius > length)
		{
			balls[i].pos.x = length - balls[i].radius;
			balls[i].speed.x = -balls[i].speed.x;
		}
		if (balls[i].pos.z - balls[i].radius < -width)
		{
			balls[i].pos.z = -width + balls[i].radius;
			balls[i].speed.z = -balls[i].speed.z;
		}
		else if (balls[i].pos.z + balls[i].radius > width)
		{
			balls[i].pos.z = width - balls[i].radius;
			balls[i].speed.z = -balls[i].speed.z;
		}
		if (balls[i].pos.y - balls[i].radius < 0)
		{
			balls[i].pos.y = balls[i].radius;
			balls[i].speed.y = -balls[i].speed.y;
		}
		else if (balls[i].pos.y + balls[i].radius > height)
		{
			balls[i].pos.y = height - balls[i].radius;
			balls[i].speed.y = -balls[i].speed.y;
		}
	}
}

__global__ void collision(uint32_t* cells, uint32_t* objects, Ball* balls, int num_balls, int num_cells, uint32_t* indices,
	uint32_t num_indices, unsigned int group_per_thread, float length, float width, float height, 
	float gridSize, int grid_x, int grid_y, int grid_z)
{
	int index = threadIdx.x + blockIdx.x * blockDim.x;
	for (int group_num = 0; group_num < group_per_thread; group_num++)
	{
		// broad phase
		// 确定开始和结束的索引
		int cell_id = index * group_per_thread + group_num;
		if (cell_id >= num_indices)
		{
			break;
		}
		int end = indices[cell_id];
		int start = 0;
		if (cell_id == 0)
		{
			start = 0;
		}
		else
		{
			start = indices[cell_id - 1];
		}

		// home的个数
		int home_num = 0;
		for (int i = start; i < end; i++)
		{
			int type = cells[i] & 1;
			if (type == HOME_CELL)
			{
				home_num++;
			}
			else
			{
				break;
			}
		}

		// 碰撞检测
		for (int i = start; i < start + home_num; i++)
		{
			if (cells[i] == UINT32_MAX) break;
			int ball_i = (objects[i] >> 1) & 65535;

			for (int j = i + 1; j < end; j++)
			{
				if (cells[j] == UINT32_MAX) break;
				int ball_j = (objects[j] >> 1) & 65535;
				
				// narrow phase
				// 直接通过球心距判断
				// 
				// 都是home 
				if (j < start + home_num)
				{
					if (isCollision(balls[ball_i], balls[ball_j]))
					{
						updateSpeed(balls[ball_i], balls[ball_j]);
					}
				}
				else
				{
					int home_i = (cells[i] >> 1) & ((1 << 24) - 1);
					int j_x = (balls[ball_j].pos.x + length) / gridSize;
					int j_y = balls[ball_j].pos.y / gridSize;
					int j_z = (balls[ball_j].pos.z + width) / gridSize;
					int home_j = j_x << 16 | j_y << 8 | j_z;

					// home和phantom
					if (home_i < home_j)
					{
						if (isCollision(balls[ball_i], balls[ball_j]))
						{
							updateSpeed(balls[ball_i], balls[ball_j]);
						}
					}
				}
			}
		}

	}
}


// 根据基数求和重新排序
__global__ void arrange(uint32_t* cells, uint32_t* objects, uint32_t* cells_temp, uint32_t* objects_temp, 
	uint32_t* radix_sums, int n, int shift)
{
	int index = threadIdx.x + blockIdx.x * blockDim.x;
	int num_radices = 1 << RADIX_LENGTH;

	if (index != 0) return;

	for (int i = 0; i < n; i++)
	{
		int current_radix_num = (cells[i] >> shift) & (num_radices - 1);
		cells_temp[radix_sums[current_radix_num]] = cells[i];
		objects_temp[radix_sums[current_radix_num]] = objects[i];
		radix_sums[current_radix_num]++;
	}
}

// 获取不同网格的起始索引及不同网格单元的数量
__global__ void getCellIndex(uint32_t* cells, int n, uint32_t* indices, uint32_t* indices_num)
{
	int index = threadIdx.x + blockIdx.x * blockDim.x;
	if (index != 0)
		return;
	indices_num[0] = 0;
	uint32_t previous = UINT32_MAX;
	uint32_t current = UINT32_MAX;
	uint32_t mask = (1 << 24) - 1;
	for (int i = 0; i < n; i++)
	{
		current = mask & (cells[i] >> 1);
		if (previous == UINT32_MAX)
		{
			previous = current;
		}
		if (previous != current)
		{
			indices[indices_num[0]] = i;
			indices_num[0]++;
		}
		previous = current;
	}
	indices[indices_num[0]] = n;
	indices_num[0]++;
}

// 基数求和，存在数组中便于后续排序
__global__ void getRadixSum(uint32_t* cells, uint32_t* radix_sums, int n, int shift)
{
	int index = threadIdx.x + blockIdx.x * blockDim.x;
	int stride = blockDim.x * gridDim.x;
	int num_indices = 1 << RADIX_LENGTH;

	for (int i = index; i < num_indices; i++)
	{
		radix_sums[i] = 0;
	}
	__syncthreads();


	// 每个线程处理对应的小球
	for (int i = index; i < n; i += stride)
	{

		for (int j = 0; j < blockDim.x; j++)
		{
			if (threadIdx.x % blockDim.x == j)
			{
				int current_radix_num = (cells[i] >> shift) & (num_indices - 1);
				radix_sums[current_radix_num]++;
			}
		}

	}
	__syncthreads();

	// 获取前缀和
	int o = 1;
	int a;

	// 归约
	for (int d = num_indices / 2; d; d /= 2)
	{
		__syncthreads();

		if (threadIdx.x < d)
		{
			a = (threadIdx.x * 2 + 1) * o - 1;
			radix_sums[a + o] += radix_sums[a];
		}

		o *= 2;
	}
	if (!threadIdx.x)
	{
		radix_sums[num_indices - 1] = 0;
	}

	// 反向传播
	uint32_t temp;
	for (int d = 1; d < num_indices; d *= 2)
	{
		__syncthreads();
		o /= 2;

		if (threadIdx.x < d)
		{
			a = (threadIdx.x * 2 + 1) * o - 1;
			temp = radix_sums[a];
			radix_sums[a] = radix_sums[a + o];
			radix_sums[a + o] += temp;
		}
	}

	__syncthreads();
}

// 初始化球体在空间网格中的位置和信息
__global__ void init(uint32_t* cells, uint32_t* objects, Ball* balls, float length, float width, float height, float gridSize, int grid_x, int grid_y,
	int grid_z, int n)
{
	unsigned int count = 0;

	// 遍历所有球体
	for (int i = blockIdx.x * blockDim.x + threadIdx.x; i < n; i += gridDim.x * blockDim.x)
	{
		// 计算所在网格信息并存入cells和objects数组
		int current_cell_id = i * 8;
		int cell_info = 0;
		int object_info = 0;
		int current_count = 0;
		float x = balls[i].pos.x;
		float y = balls[i].pos.y;
		float z = balls[i].pos.z;
		float radius = balls[i].radius;

		int hash_x = (x + length) / gridSize;
		int hash_y = (y) / gridSize;
		int hash_z = (z + width) / gridSize;
		cell_info = hash_x << 17 | hash_y << 9 | hash_z << 1 | HOME_CELL;
		object_info = i << 1 | HOME_OBJECT;
		cells[current_cell_id] = cell_info;
		objects[current_cell_id] = object_info;
		current_cell_id++;
		count++;
		current_count++;

		// 查找记录相邻的单元格
		for (int dx = -1; dx <= 1; dx++)
		{
			for (int dy = -1; dy <= 1; dy++)
			{
				for (int dz = -1; dz <= 1; dz++)
				{
					int new_hash_x = hash_x + dx;
					int new_hash_y = hash_y + dy;
					int new_hash_z = hash_z + dz;

					// 自己
					if (dx == 0 && dy == 0 && dz == 0)
					{
						continue;
					}

					// 越界
					if (new_hash_x < 0 || new_hash_x >= grid_x ||
						new_hash_y < 0 || new_hash_y >= grid_y ||
						new_hash_z < 0 || new_hash_z >= grid_z)
					{
						continue;
					}

					float relative_x = 0;
					float relative_y = 0;
					float relative_z = 0;
					if (dx == 0)
					{
						relative_x = x;
					}
					else if (dx == -1)
					{
						relative_x = hash_x * gridSize - length;
					}
					else
					{
						relative_x = (hash_x + 1) * gridSize - length;
					}

					if (dz == 0)
					{
						relative_z = z;
					}
					else if (dz == -1)
					{
						relative_z = hash_z * gridSize - width;
					}
					else
					{
						relative_z = (hash_z + 1) * gridSize - width;
					}

					if (dy == 0)
					{
						relative_y = y;
					}
					else if (dy == -1)
					{
						relative_y = hash_y * gridSize;
					}
					else
					{
						relative_y = (hash_y + 1) * gridSize;
					}

					relative_x -= x;
					relative_y -= y;
					relative_z -= z;

					float dist = dist_(relative_x, relative_y, relative_z);
					if (dist < radius)
					{
						int cell_info = new_hash_x << 17 | new_hash_y << 9 | new_hash_z << 1 | PHANTOM_CELL;
						int object_info = i << 1 | PHANTOM_OBJECT;
						cells[current_cell_id] = cell_info;
						objects[current_cell_id] = object_info;
						current_cell_id++;
						count++;
						current_count++;
					}
				}
			}
		}

		// 每个球体的信息应该占据连续的8个位置，空余的要补齐
		while (current_count < 8)
		{

			cells[current_cell_id] = UINT32_MAX;
			objects[current_cell_id] = i << 2;
			current_cell_id++;
			current_count++;
		}

	}

}


void collisionDetection(Ball* balls, float refreshInterval, float length, float width, float height,
	float gridSize, int grid_x, int grid_y, int grid_z, int n)
{

	// GPU上的小球数组
	Ball* g_balls;
	unsigned int nBytes = n * sizeof(Ball);
	cudaMalloc((void**)&g_balls, nBytes);

	unsigned int num_blocks = 128;
	unsigned int threads_per_block = 512;
	unsigned int object_size = (n - 1) / threads_per_block + 1;
	if (object_size < num_blocks) {
		num_blocks = object_size;
	}

	// 将cpu数据复制到gpu上
	cudaMemcpy((void*)g_balls, (void*)balls, nBytes, cudaMemcpyHostToDevice);

	// 更新状态
	updateBalls <<< num_blocks, threads_per_block >>> (g_balls, refreshInterval, length, width, height, n);
	cudaDeviceSynchronize();

	// 碰撞检测
	spatialSubdivision(g_balls, length, width, height, gridSize, grid_x, grid_y, grid_z, n, num_blocks, threads_per_block);
	cudaDeviceSynchronize();

	// 计算好的结果复制回cpu，更新图形界面
	cudaMemcpy((void*)balls, (void*)g_balls, nBytes, cudaMemcpyDeviceToHost);

	cudaFree(g_balls);
}

void spatialSubdivision(Ball* balls, float length, float width, float height,
	float gridSize, int grid_x, int grid_y, int grid_z, int n, unsigned int num_blocks, unsigned int threads_per_block)
{
	unsigned int cell_size = n * 8 * sizeof(uint32_t);

	int num_radices = 1 << RADIX_LENGTH;
	uint32_t* cells;
	uint32_t* cells_temp;
	uint32_t* objects;
	uint32_t* objects_temp;
	uint32_t* indices;
	uint32_t* indices_num;
	uint32_t* radix_sums;

	cudaMalloc((void**)&cells, cell_size);
	cudaMalloc((void**)&cells_temp, cell_size);
	cudaMalloc((void**)&objects, cell_size);
	cudaMalloc((void**)&objects_temp, cell_size);
	cudaMalloc((void**)&indices, cell_size);
	cudaMalloc((void**)&indices_num, sizeof(uint32_t));
	cudaMalloc((void**)&radix_sums, num_radices * sizeof(uint32_t));

	// initialize cells and objects
	init <<< num_blocks, threads_per_block, threads_per_block * sizeof(unsigned int) >>> (cells, objects, balls, length, width, height, gridSize, grid_x, grid_y, grid_z, n);

	// in-place排序，将H cell排在P cell前面
	radixSort(cells, objects, cells_temp, objects_temp, radix_sums, 8 * n, indices, indices_num, num_blocks, threads_per_block);

	uint32_t indices_num_;
	cudaMemcpy((void*)&indices_num_, (void*)indices_num, sizeof(uint32_t), cudaMemcpyDeviceToHost);

	unsigned int threads_total = num_blocks * threads_per_block;
	unsigned int group_per_thread = indices_num_ / threads_total + 1;
	collision <<< num_blocks, threads_per_block >>> (cells, objects, balls, n, 8 * n, indices, indices_num_, group_per_thread, length, width, height, gridSize, grid_x, grid_y, grid_z);

	cudaFree(cells);
	cudaFree(cells_temp);
	cudaFree(objects);
	cudaFree(objects_temp);
	cudaFree(indices);
	cudaFree(indices_num);
	cudaFree(radix_sums);
}

// 基数排序
void radixSort(uint32_t* cells, uint32_t* objects, uint32_t* cells_temp, uint32_t* objects_temp,
	uint32_t* radix_sums, int n, uint32_t* indices, uint32_t* num_indices, unsigned int num_blocks, 
	unsigned int threads_per_block)
{
	for (int i = 0; i < 32; i += RADIX_LENGTH)
	{
		getRadixSum <<< num_blocks, threads_per_block >>> (cells, radix_sums, n, i);

		arrange <<< num_blocks, threads_per_block >>> (cells, objects, cells_temp, objects_temp, radix_sums, n, i);
		
		uint32_t* cells_s = cells;
		cells = cells_temp;
		cells_temp = cells_s;

		uint32_t* objects_s = objects;
		objects = objects_temp;
		objects_temp = objects_s;
	}

	getCellIndex <<< num_blocks, threads_per_block >>> (cells, n, indices, num_indices);
}