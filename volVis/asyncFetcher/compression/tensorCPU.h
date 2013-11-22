

#ifndef TENSOR_CPU__H
#define TENSOR_CPU__H

#include <msv/types/types.h>

namespace cpu
{

void decompressTensorBasic(   void*     dst,
                              float*    tmp1,
                              float*    tmp2,
                        const float*    core,
                        const float*    u1,
                        const float*    u2,
                        const float*    u3,
                              int       coreDim,
                              int       blockDim,
                              int       uStride,
                              float     scale,
                              byte      nBytes  );

void decompressTensorQuantized(
                              void*     dst,
                              float*    tmp1,
                              float*    tmp2,
                        const byte*     core,
                        const float*    u1,
                        const float*    u2,
                        const float*    u3,
                              int       coreDim,
                              int       blockDim,
                              int       uStride,
                              float     scale,
                              byte      nBytes  );


void decompressTensorQuantized2(
                              void*     dst,
                              float*    tmp1,
                              float*    tmp2,
                        const byte*     core,
                        const float*    u1,
                        const float*    u2,
                        const float*    u3,
                              int       coreDim,
                              int       rank,
                              int       blockDim,
                              int       uStride,
                              float     scale,
                              byte      nBytes  );

/**
 * Slow reference version.
 */
void decompressTensorRefference(
                              void*     dst,
                        const float*    core,
                        const float*    u1,
                        const float*    u2,
                        const float*    u3,
                              int       coreDim,
                              int       blockDim,
                              int       uStride,
                              float     scale,
                              byte      nBytes );


void saveTensorResult(  const byte*     dst,
                        const float*    core,
                        const float*    u1,
                        const float*    u2,
                        const float*    u3,
                              int       coreDim,
                              int       blockDim,
                              int       uStride,
                              float     scale,
                              byte      nBytes );
}


#endif // TENSOR_CPU__H
