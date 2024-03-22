#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "CudaSimHandler.cuh"

#include <stdio.h>
#include <iostream>

cudaError_t simWithCuda(CellState* cellGrid, int sizeX, int sizeY, int steps);

__global__ void cellSimKernel(CellState* inGrid, CellState* outGrid, int sizeX, int sizeY)
{
	int x = blockIdx.x * CELLBLOCKSIZEX + threadIdx.x;
	int y = blockIdx.y * CELLBLOCKSIZEY + threadIdx.y;
	int i = x * sizeX + y;
	if (x == 0 || x == sizeY-1 || y == 0 || y == sizeX-1) {
		outGrid[i] = DEAD;
		return;
	}
 
	int alive = inGrid[i - 1];
	alive += inGrid[i + 1];
	alive += inGrid[i - sizeX];
	alive += inGrid[i - sizeX + 1];
	alive += inGrid[i - sizeX - 1];
	alive += inGrid[i + sizeX];
	alive += inGrid[i + sizeX + 1];
	alive += inGrid[i + sizeX - 1];

	if (inGrid[i] == DEAD) { 
		if (alive != ANIMATEDEAD) {
			outGrid[i] = DEAD;
			return;
		}
	}
	else {
		if (alive < ISOLATION || alive > SMOTHERING) {
			outGrid[i] = DEAD;
			return;
		}
	}
	outGrid[i] = ALIVE;
}

int SimStep(CellState* cellGrid, vec size, int steps) {
	cudaError_t cudaStatus = simWithCuda(cellGrid, size.x, size.y, steps);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "simWithCuda failed!");
		return 1;
	}
	return 0;
}

cudaError_t simWithCuda(CellState* cellGrid, int sizeX, int sizeY, int steps) {

	CellState* dev_inGrid;
	CellState* dev_outGrid;
	cudaError_t cudaStatus;

	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
		goto Error;
	}

	int size = sizeX * sizeY;

	// Allocate GPU buffers
	cudaStatus = cudaMalloc((void**)&dev_inGrid, sizeof(CellState) * size);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		return cudaStatus;
	}
	cudaStatus = cudaMalloc((void**)&dev_outGrid, sizeof(CellState) * size);
	if (cudaStatus != cudaSuccess) {
		cudaFree(dev_inGrid);
		fprintf(stderr, "cudaMalloc failed!");
		return cudaStatus;
	}

	// Copy the cellGrid to the gpu
	cudaStatus = cudaMemcpy(dev_inGrid, cellGrid, sizeof(CellState) * size, cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

	dim3 blocks(sizeX / CELLBLOCKSIZEX, sizeY / CELLBLOCKSIZEY);
	dim3 threads(CELLBLOCKSIZEX, CELLBLOCKSIZEY);

	//Handle multiple possible simsteps
	for (size_t i = 0; i < steps; i++) {

		if (i != 0) {
			CellState* temp = dev_inGrid;
			dev_inGrid = dev_outGrid;
			dev_outGrid = temp;
		}

		// Launch a kernel on the GPU
		cellSimKernel << <blocks, threads >> > (dev_inGrid, dev_outGrid, sizeX, sizeY);

		// Check for any errors launching the kernel
		cudaStatus = cudaGetLastError();
		if (cudaStatus != cudaSuccess) {
			fprintf(stderr, "cellSimKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
			goto Error;
		}

		// cudaDeviceSynchronize waits for the kernel to finish, and returns any errors encountered during the launch.
		cudaStatus = cudaDeviceSynchronize();
		if (cudaStatus != cudaSuccess) {
			fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
			goto Error;
		}
	}

	// Copy output vector from GPU buffer to host memory.
	cudaStatus = cudaMemcpy(cellGrid, dev_outGrid, sizeof(CellState) * size, cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

Error:
	//Free the gpu allocations
	cudaFree(dev_inGrid);
	cudaFree(dev_outGrid);

	return cudaStatus;
}

//Called at the end of the program
int CudaCleanup() {
	// cudaDeviceReset must be called before exiting in order for profiling and
	// tracing tools such as Nsight and Visual Profiler to show complete traces.
	if (cudaDeviceReset() != cudaSuccess) {
		fprintf(stderr, "cudaDeviceReset failed!");
		return 1;
	}
	return 0;
}
