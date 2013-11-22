
#include "cudaDecompressorKernel.cu"

#include <cstdlib>
#include <cstdio>
#include <string.h>

#ifdef _WIN32
#  include <windows.h>
#endif

#include <cuda_gl_interop.h>

extern "C"
{

#define PRINT_TENSOR_TIMINGS

void checkCUDAError( const char *msg )
{
    cudaError_t err = cudaGetLastError();
    if( cudaSuccess != err) 
    {
        fprintf(stderr, "Cuda error: %s: %s.\n", msg, cudaGetErrorString( err ));
        exit(EXIT_FAILURE);
    }
}


void allocateDeviceArray( void** array, int numBytes )
{
    cudaMalloc( array, numBytes );
}


void deleteDeviceArray( void** array )
{
    cudaFree( *array );
    *array = 0;
}


void copyArrayToDevice( void* device, const void* host, int numBytes )
{
    cudaMemcpy( device, host, numBytes, cudaMemcpyHostToDevice );
}

void copyArrayToHost( void* host, const void* device, int numBytes )
{
    cudaMemcpy( host, device, numBytes, cudaMemcpyDeviceToHost);
}


void registerGLBufferObject( unsigned int pbo )
{
    cudaGLRegisterBufferObject( pbo );
}


void unregisterGLBufferObject( unsigned int pbo )
{
    cudaGLUnregisterBufferObject( pbo );
}


void cuDecompTensor( unsigned char bytes,
                          void*    dst,
                    const float*   core,
                          float*   tmp1,
                          float*   tmp2,
                    const float*   u1,
                    const float*   u2,
                    const float*   u3,
                          int      coreDim,
                          int      blockDim,
                          int      uStride,
                          float    scale )
{
#ifdef PRINT_TENSOR_TIMINGS
    cudaEvent_t start, stage1, stage2, stop;
    cudaEventCreate( &start  );
    cudaEventCreate( &stage1 );
    cudaEventCreate( &stage2 );
    cudaEventCreate( &stop   );
    cudaEventRecord( start, 0 );
#endif

    dim3 threadD1( BLOCK_GRID, BLOCK_GRID );
    dim3 blockD1( ( coreDim+BLOCK_GRID-1)/BLOCK_GRID,
                  (blockDim+BLOCK_GRID-1)/BLOCK_GRID,
                    coreDim );
    cuDecompTensor_1<<< blockD1, threadD1 >>>( tmp1, core, u1, coreDim, blockDim, uStride );
#ifdef PRINT_TENSOR_TIMINGS
    cudaEventRecord( stage1, 0 );
#endif

    dim3 threadD2( BLOCK_GRID, BLOCK_GRID );
    dim3 blockD2( (blockDim+BLOCK_GRID-1)/BLOCK_GRID,
                  (blockDim+BLOCK_GRID-1)/BLOCK_GRID,
                    coreDim );
    cuDecompTensor_2<<< blockD2, threadD2 >>>( tmp2, tmp1, u2, coreDim, blockDim, uStride );
#ifdef PRINT_TENSOR_TIMINGS
    cudaEventRecord( stage2, 0 );
#endif

    dim3 threadD3( BLOCK_GRID, BLOCK_GRID );
    dim3 blockD3( (blockDim+BLOCK_GRID-1)/BLOCK_GRID,
                  (blockDim+BLOCK_GRID-1)/BLOCK_GRID,
                   blockDim );
    if( bytes == 1 )
        cuDecompTensor_3b<<< blockD3, threadD3 >>>( (unsigned  char*)dst, tmp2, u3, coreDim, blockDim, uStride, scale );
    else
        cuDecompTensor_3s<<< blockD3, threadD3 >>>( (unsigned short*)dst, tmp2, u3, coreDim, blockDim, uStride, scale );

#ifdef PRINT_TENSOR_TIMINGS
    cudaEventRecord( stop, 0 );
    cudaEventSynchronize( stop );
    float elapsed1;
    float elapsed2;
    float elapsed3;
    float elapsedT;
    cudaEventElapsedTime( &elapsed1, start,  stage1 );
    cudaEventElapsedTime( &elapsed2, stage1, stage2 );
    cudaEventElapsedTime( &elapsed3, stage2, stop   );
    cudaEventElapsedTime( &elapsedT, start,  stop   );
    printf( " Time Total: %3.3f ms (%3.3f fps)\n", elapsedT, 1000.0f / elapsedT );
    printf( "     stage1: %3.3f ms (%3.3f fps)\n", elapsed1, 1000.0f / elapsed1 );
    printf( "     stage2: %3.3f ms (%3.3f fps)\n", elapsed2, 1000.0f / elapsed2 );
    printf( "     stage3: %3.3f ms (%3.3f fps)\n", elapsed3, 1000.0f / elapsed3 );
#endif
}



void _printDevProp( cudaDeviceProp* devProp )
{
    printf("Major revision number:         %d\n",  devProp->major);
    printf("Minor revision number:         %d\n",  devProp->minor);
    printf("Name:                          %s\n",  devProp->name);
    printf("Total global memory:           %lu\n", devProp->totalGlobalMem);
    printf("Total shared memory per block: %lu\n", devProp->sharedMemPerBlock);
    printf("Total registers per block:     %d\n",  devProp->regsPerBlock);
    printf("Warp size:                     %d\n",  devProp->warpSize);
    printf("Maximum memory pitch:          %lu\n", devProp->memPitch);
    printf("Maximum threads per block:     %d\n",  devProp->maxThreadsPerBlock);
    for (int i = 0; i < 3; ++i)
    printf("Maximum dimension %d of block: %d\n", i, devProp->maxThreadsDim[i]);
    for (int i = 0; i < 3; ++i)
    printf("Maximum dimension %d of grid:  %d\n", i, devProp->maxGridSize[i]);
    printf("Clock rate:                    %d\n",  devProp->clockRate);
    printf("Total constant memory:         %lu\n", devProp->totalConstMem);
    printf("Texture alignment:             %lu\n", devProp->textureAlignment);
    printf("Concurrent copy and execution: %s\n",  (devProp->deviceOverlap ? "Yes" : "No"));
    printf("Number of multiprocessors:     %d\n",  devProp->multiProcessorCount);
    printf("Kernel execution timeout:      %s\n",  (devProp->kernelExecTimeoutEnabled ? "Yes" : "No"));
}

void printCUDAInfo()
{
    int devCount;
    cudaGetDeviceCount( &devCount );
    printf( "CUDA Device Query...\n" );
    printf( "There are %d CUDA devices.\n", devCount );

    for(int i = 0; i < devCount; ++i )
    {
        printf ("\nCUDA Device #%d\n", i );
        cudaDeviceProp devProp;
        cudaGetDeviceProperties( &devProp, i );
        _printDevProp( &devProp );
    }
}




}









