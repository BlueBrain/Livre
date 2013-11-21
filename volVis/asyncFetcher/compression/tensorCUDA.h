

#ifndef MASS_VOL__CUDA_DECOMPRESSOR_H
#define MASS_VOL__CUDA_DECOMPRESSOR_H

#ifdef EQ_USE_CUDA
#include <cuda.h>
#include <stdint.h>

extern "C"
{
    void checkCUDAError( const char *msg );

    void allocateDeviceArray( void** array, int numBytes );
    void deleteDeviceArray(   void** array );

    void copyArrayToDevice( void* device, const void* host,   int numBytes );
    void copyArrayToHost(   void* host,   const void* device, int numBytes );

    void registerGLBufferObject( unsigned int pbo );
    void unregisterGLBufferObject( unsigned int pbo );

    void cudaDecompTensor( unsigned int  pboDst,
                                float*   tmp1,
                                float*   tmp2,
                          const float*   core,
                          const float*   u1,
                          const float*   u2,
                          const float*   u3,
                                int      coreDim,
                                int      blockDim,
                                int      uStride,
                                float    scale,
                           unsigned char bytes );

    void cudaDecompTensorQuantized(
                                unsigned int  pboDst,
                                float*   tmp1,
                                float*   tmp2,
                          const uint8_t* core,
                          const float*   u1,
                          const float*   u2,
                          const float*   u3,
                                int      coreDim,
                                int      rank,
                                int      blockDim,
                                int      uStride,
                                float    scale,
                           unsigned char bytes );
}
#else
namespace
{
    void checkCUDAError( const char* ){}

    void allocateDeviceArray( void**, int ){}
    void deleteDeviceArray(   void** ){}

    void copyArrayToDevice( void*, const void*, int ){}
    void copyArrayToHost(   void*, const void*, int ){}

    void registerGLBufferObject( unsigned int ){}
    void unregisterGLBufferObject( unsigned int ){}

    void cudaDecompTensor( unsigned int,
                                float*,
                                float*,
                          const float*,
                          const float*,
                          const float*,
                          const float*,
                                int,
                                int,
                                int,
                                float,
                           unsigned char ){}

    void cudaDecompTensorQuantized(
                                unsigned int  pboDst,
                                float*   tmp1,
                                float*   tmp2,
                          const uint8_t* core,
                          const float*   u1,
                          const float*   u2,
                          const float*   u3,
                                int      coreDim,
                                int      rank,
                                int      blockDim,
                                int      uStride,
                                float    scale,
                           unsigned char bytes ) {}
}
#endif

#endif// MASS_VOL__CUDA_DECOMPRESSOR_H
