
#include "cpuDecompressor.h"
#include <msv/util/hlp.h>
#include <msv/types/limits.h>

namespace cpu
{

void decompressTensor1(       float*    dst,
                        const float*    core,
                        const byte      coreDim,
                        const float*    u1,
                        const uint32_t  blockDim,
                        const uint32_t  uStride
                       )
{
    const uint32_t R    = coreDim;
    const uint32_t RR   = R*R;
    const uint32_t I    = blockDim;
    const uint32_t RI   = R * I;
#pragma omp parallel for
    for( uint32_t z = 0; z < R; ++z )                    //   top z: 0..R
    {
        const float* uL = u1;
        for( uint32_t y = 0; y < I; ++y )                //   top y: 0..I
        {
            for( uint32_t x = 0; x < R; ++x )            //   top x: 0..R
            {
                double sum = 0;
                const float* c = core + x*RR + z*R;
                for( uint32_t i = 0; i < R; ++i )        // inner i: 0..R
                {
                    sum += uL[i]*c[i];
                }
                dst[z + y*R + x*RI] = sum;
            }
            uL += uStride;
        }
    }
}


void decompressTensor2(       float*    dst,
                        const float*    src,
                        const byte      coreDim,
                        const float*    u2,
                        const uint32_t  blockDim,
                        const uint32_t  uStride
                       )
{
    const uint32_t R    = coreDim;
    const uint32_t I    = blockDim;
    const uint32_t RI   = R * I;

//    _transpose( tmp, src, Vec3_ui32( R, I, R ));

#pragma omp parallel for
    for( uint32_t z = 0; z < R; ++z )                    //   top z: 0..R
    {
        const float* uL = u2;
        for( uint32_t y = 0; y < I; ++y )                //   top y: 0..I
        {
            for( uint32_t x = 0; x < I; ++x )            //   top x: 0..I
            {
                double sum = 0;
                const float* c = src + x*R + z*RI;
                for( uint32_t i = 0; i < R; ++i )            // inner i: 0..R
                {
                    sum += uL[i]*c[i];
                }
                dst[z + y*R + x*RI] = sum;
            }
            uL += uStride;
        }
    }
}


template< typename T >
void _decompressTensor3(          T*    dst,
                        const float*    src,
                        const byte      coreDim,
                        const float*    u3,
                        const uint32_t  blockDim,
                        const uint32_t  uStride,
                        const float     scale
                       )
{
    const double maxSum = massVolVis::Limits<T>::max();

    const uint32_t R    = coreDim;
    const uint32_t I    = blockDim;
    const uint32_t II   = I * I;
    const uint32_t RI   = R * I;

#pragma omp parallel for
    for( uint32_t z = 0; z < I; ++z )                    //   top z: 0..I
    {
        const float* uL = u3;
        for( uint32_t y = 0; y < I; ++y )                //   top y: 0..I
        {
            for( uint32_t x = 0; x < I; ++x )            //   top x: 0..I
            {
                double sum = 0;
                const float* c = src + x*R + z*RI;
                for( uint32_t i = 0; i < R; ++i )           // inner i: 0..R
                {
                    sum += uL[i]*c[i];
                }
                dst[y*II + x*I + z] =  hlpFuncs::myClip<double>( sum*scale, 0., maxSum );
            }
            uL += uStride;
        }
    }
}


void decompressTensor3(       byte*     dst,
                        const float*    src,
                        const byte      coreDim,
                        const float*    u3,
                        const uint32_t  blockDim,
                        const uint32_t  uStride,
                        const float     scale,
                        const byte      nBytes
                       )
{
    if( nBytes )
        _decompressTensor3( dst, src, coreDim, u3, blockDim, uStride, scale );
    else
        _decompressTensor3( reinterpret_cast<uint16_t*>(dst), src, coreDim, u3, blockDim, uStride, scale );
}
}

