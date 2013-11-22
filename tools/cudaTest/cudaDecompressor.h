

#ifndef MASS_VOL__CUDA_DECOMPRESSOR_H
#define MASS_VOL__CUDA_DECOMPRESSOR_H

#include <eq/eq.h>

#ifdef EQ_USE_CUDA
#include <cuda.h>

extern "C"
{
    void checkCUDAError( const char *msg );

    void allocateDeviceArray( void** array, int numBytes );
    void deleteDeviceArray(   void** array );

    void copyArrayToDevice( void* device, const void* host,   int numBytes );
    void copyArrayToHost(   void* host,   const void* device, int numBytes );

    void registerGLBufferObject(   unsigned int pbo );
    void unregisterGLBufferObject( unsigned int pbo );


    void cuDecompTensor( unsigned char bytes,
                                 void*   dst,
                          const float*   core,
                                float*   tmp1,
                                float*   tmp2,
                          const float*   u1,
                          const float*   u2,
                          const float*   u3,
                                int      coreDim,
                                int      blockDim,
                                int      uStride,
                                float    scale );

    void printCUDAInfo();
}
#else
namespace
{
    void checkCUDAError( const char * ){ frintf( "CUDA is not available" ); }

    void allocateDeviceArray( void**, int ){}
    void deleteDeviceArray(   void** ){}

    void copyArrayToDevice( void*, const void*, int ){}
    void copyArrayToHost(   void*, const void*, int ){}

    void registerGLBufferObject( unsigned int ){}
    void unregisterGLBufferObject( unsigned int ){}

    void cuDecompTensor( unsigned char,
                                 void*,
                          const float*,
                                float*,
                                float*,
                          const float*,
                          const float*,
                          const float*,
                                int,
                                int,
                                int,
                                float ){}

    void printCUDAInfo(){}
}
#endif

#endif// MASS_VOL__CUDA_DECOMPRESSOR_H
