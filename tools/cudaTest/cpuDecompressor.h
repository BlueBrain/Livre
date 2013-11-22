

#ifndef CPU_DECOMPRESSOR__H
#define CPU_DECOMPRESSOR__H

#include <msv/types/types.h>

namespace cpu
{

void decompressTensor1(       float*    dst,
                        const float*    core,
                        const byte      coreDim,
                        const float*    u1,
                        const uint32_t  blockDim,
                        const uint32_t  uStride
                       );

void decompressTensor2(       float*    dst,
                        const float*    src,
                        const byte      coreDim,
                        const float*    u2,
                        const uint32_t  blockDim,
                        const uint32_t  uStride
                       );

void decompressTensor3(       byte*     dst,
                        const float*    src,
                        const byte      coreDim,
                        const float*    u3,
                        const uint32_t  blockDim,
                        const uint32_t  uStride,
                        const float     scale,
                        const byte      nBytes
                       );


}


#endif // CPU_DECOMPRESSOR__H
