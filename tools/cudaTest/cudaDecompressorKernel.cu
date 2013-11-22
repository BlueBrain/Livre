

#ifndef MASS_VOL__CUDA_DECOMPRESSOR_KERNELL_H
#define MASS_VOL__CUDA_DECOMPRESSOR_KERNELL_H

#include <math.h>

#ifdef __DEVICE_EMULATION__
#include <stdio.h>
#endif

#define BLOCK_GRID 16

__global__ void cuDecompTensor_1(        float*   dst,
                                   const float*   core,
                                   const float*   u1,
                                   int coreD,
                                   int blockD,
                                   int uStride )
{
    int i, j;
    int R  = coreD;
    int RR = R*R;
    int I  = blockD;
    int RI = R * I;

    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z;

    bool pass = x < R && y < I && z < R;

    float sum = 0;
    __shared__ float uS[BLOCK_GRID][BLOCK_GRID];
    __shared__ float cS[BLOCK_GRID][BLOCK_GRID];
    for( i = 0; i < R; i+= BLOCK_GRID )
    {
        if( y < I && i+threadIdx.y < R )
            uS[threadIdx.y][threadIdx.x] = u1[  y*uStride  + i+threadIdx.x];
        else
            uS[threadIdx.y][threadIdx.x] = 0; // required to avoid "if" in the loop below

        if( x < R && i+threadIdx.x < R )
            cS[threadIdx.y][threadIdx.x] = core[x*RR + z*R + i+threadIdx.y];
        else
            cS[threadIdx.y][threadIdx.x] = 0;

        __syncthreads();
        if( pass )
        {
            for( j = 0; j < BLOCK_GRID; ++j )
                sum += uS[threadIdx.y][j]*cS[j][threadIdx.x];
        }
        __syncthreads();
    }
    if( pass )
        dst[ z + y*R + x*RI ] = sum;
}

__global__ void cuDecompTensor_2(        float*   dst,
                                   const float*   src,
                                   const float*   u2,
                                   int coreD,
                                   int blockD,
                                   int uStride )
{
    int i, j;
    int R  = coreD;
    int I  = blockD;
    int RI = R * I;

    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z;

    bool pass = x < I && y < I && z < R;

    float sum = 0;
    __shared__ float uS[BLOCK_GRID][BLOCK_GRID];
    __shared__ float cS[BLOCK_GRID][BLOCK_GRID];
    for( i = 0; i < R; i+= BLOCK_GRID )
    {
        if( y < I && i+threadIdx.y < R )
            uS[threadIdx.y][threadIdx.x] = u2[  y*uStride  + i+threadIdx.x];
        else
            uS[threadIdx.y][threadIdx.x] = 0; // required to avoid "if" in the loop below

        if( x < I && i+threadIdx.x < R )
            cS[threadIdx.y][threadIdx.x] = src[x*R + z*RI + i+threadIdx.y];
        else
            cS[threadIdx.y][threadIdx.x] = 0;

        __syncthreads();
        if( pass )
        {
            for( j = 0; j < BLOCK_GRID; ++j )
                sum += uS[threadIdx.y][j]*cS[j][threadIdx.x];
        }
        __syncthreads();
    }
    if( pass )
        dst[ z + y*R + x*RI ] = sum;
}


__global__ void cuDecompTensor_3b(       unsigned char*   dst,
                                   const float*   src,
                                   const float*   u3,
                                   int   coreD,
                                   int   blockD,
                                   int   uStride,
                                   float scale )
{
    int i, j;
    int R  = coreD;
    int I  = blockD;
    int II = I * I;
    int RI = R * I;

    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z;

    bool pass = x < I && y < I && z < I;

    float sum = 0;
    __shared__ float uS[BLOCK_GRID][BLOCK_GRID];
    __shared__ float cS[BLOCK_GRID][BLOCK_GRID];
    for( i = 0; i < R; i+= BLOCK_GRID )
    {
        if( y < I && i+threadIdx.y < R )
            uS[threadIdx.y][threadIdx.x] = u3[  y*uStride  + i+threadIdx.x];
        else
            uS[threadIdx.y][threadIdx.x] = 0; // required to avoid "if" in the loop below

        if( x < I && i+threadIdx.x < R )
            cS[threadIdx.y][threadIdx.x] = src[x*R + z*RI + i+threadIdx.y];
        else
            cS[threadIdx.y][threadIdx.x] = 0;

        __syncthreads();
        if( pass )
        {
            for( j = 0; j < BLOCK_GRID; ++j )
                sum += uS[threadIdx.y][j]*cS[j][threadIdx.x];
        }
        __syncthreads();
    }
    if( pass )
        dst[ z + y*II + x*I ] = fmin( fmax( sum*scale, 0.f), 255.f );
}

//untested
__global__ void cuDecompTensor_3s( unsigned short*   dst,
                                   const float*   src,
                                   const float*   u3,
                                   int   coreD,
                                   int   blockD,
                                   int   uStride,
                                   float scale )
{
    int i, j;
    int R  = coreD;
    int I  = blockD;
    int II = I * I;
    int RI = R * I;

    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z;

    bool pass = x < I && y < I && z < I;

    float sum = 0;
    __shared__ float uS[BLOCK_GRID][BLOCK_GRID];
    __shared__ float cS[BLOCK_GRID][BLOCK_GRID];
    for( i = 0; i < R; i+= BLOCK_GRID )
    {
        if( y < I && i+threadIdx.y < R )
            uS[threadIdx.y][threadIdx.x] = u3[  y*uStride  + i+threadIdx.x];
        else
            uS[threadIdx.y][threadIdx.x] = 0; // required to avoid "if" in the loop below

        if( x < I && i+threadIdx.x < R )
            cS[threadIdx.y][threadIdx.x] = src[x*R + z*RI + i+threadIdx.y];
        else
            cS[threadIdx.y][threadIdx.x] = 0;

        __syncthreads();
        if( pass )
        {
            for( j = 0; j < BLOCK_GRID; ++j )
                sum += uS[threadIdx.y][j]*cS[j][threadIdx.x];
        }
        __syncthreads();
    }
    if( pass )
        dst[ z + y*II + x*I ] = fmin( fmax( sum*scale, 0.f), 65535.f );
}

#endif // MASS_VOL__CUDA_DECOMPRESSOR_KERNELL_H
