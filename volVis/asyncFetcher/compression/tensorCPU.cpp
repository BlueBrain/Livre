
#include "tensorCPU.h"
#include <msv/util/hlp.h>
#include <msv/types/limits.h>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <cstdlib>

#ifdef _OPENMP
    #include <omp.h>
#endif

#include <math.h> // exp2f

namespace cpu
{

const std::string _pathSrt = "/home/maxus/tmp/compressionTst.dat";

void saveTensorResult(  const byte*     dst,
                        const float*    core,
                        const float*    u1,
                        const float*    u2,
                        const float*    u3,
                              int       coreDim,
                              int       blockDim,
                              int       uStride,
                              float     scale,
                              byte      nBytes )
{
    std::cout << "Saving compression result file: " << _pathSrt.c_str() << std::endl
              << "  nBytes:   " << (int)nBytes   << std::endl
              << "  coreDim:  " <<      coreDim  << std::endl
              << "  blockDim: " <<      blockDim << std::endl
              << "  scale:    " <<      scale    << std::endl;

    const int32_t resultSizeTotal = blockDim*blockDim*blockDim;

    std::ofstream os;
    os.open( _pathSrt.c_str(), std::ios_base::out | std::ios::binary | std::ios_base::trunc );
    if( !os.is_open( ))
    {
        std::cerr << "Can't open file to read: " << _pathSrt.c_str() << std::endl;
        return;
    }

    os.write( reinterpret_cast<const char*>(&nBytes  ), sizeof(nBytes  ) );
    os.write( reinterpret_cast<const char*>(&coreDim ), sizeof(coreDim ) );
    os.write( reinterpret_cast<const char*>(&blockDim), sizeof(blockDim) );
    os.write( reinterpret_cast<const char*>(&scale   ), sizeof(scale   ) );

    const size_t cSize = coreDim*coreDim*coreDim*sizeof( core[0] );
    const size_t rSize = resultSizeTotal*nBytes;

    os.write( reinterpret_cast<const char*>( core ), cSize );
    for( byte i = 0; i < blockDim; ++i )
    {
        os.write( reinterpret_cast<const char*>( u1 ), coreDim*sizeof(u1[0]) );
        os.write( reinterpret_cast<const char*>( u2 ), coreDim*sizeof(u2[0]) );
        os.write( reinterpret_cast<const char*>( u3 ), coreDim*sizeof(u3[0]) );
        u1 += uStride;
        u2 += uStride;
        u3 += uStride;
    }
    os.write( reinterpret_cast<const char*>( &dst[0]), rSize );
    os.close();
}


/**
 * Slowest reference implementation of Tensor decompression.
 */
template< typename T >
void _decompressTensor(       T*        dst,
                        const float*    core,
                        const float*    u1,
                        const float*    u2,
                        const float*    u3,
                              int       coreDim,
                              int       blockDim,
                              int       uStride,
                              float     scale   )
{
    const double maxSum = static_cast<double>( massVolVis::Limits<T>::max() );
    std::cout << " __dZ: ";
#pragma omp parallel for
    for( int dZ = 0; dZ < blockDim; ++dZ )
    {
        std::cout << dZ << " " << std::flush;

        const float* u3L = u3 + dZ*uStride;
        const float* u2L = u2;
        for( int dY = 0; dY < blockDim; ++dY )
        {
            const float* u1L = u1;
            for( int dX = 0; dX < blockDim; ++dX )
            {
                double sum = 0;

                const float* coreL = &core[0];
                for( int cZ = 0; cZ < coreDim; ++cZ )
                for( int cY = 0; cY < coreDim; ++cY )
                {
                    const double cYZ = u2L[cY] * u3L[cZ];
                    for( int cX = 0; cX < coreDim; ++cX )
                    {
                        sum += (*coreL) * u1L[cX] * cYZ;
                        coreL++;
                    }
                }
                sum = hlpFuncs::myClip<double>( scale*sum, 0., maxSum );

                u1L += uStride;

                const uint32_t dstP = (dZ*blockDim + dY)*blockDim + dX;
                dst[ dstP ] = sum;
            }
            u2L += uStride;
        }
    }
    static int bl = 0;
    std::cout << " DONE Decompression " << (++bl) << std::endl;
}


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
                              byte      nBytes )
{
    assert( nBytes == 1 || nBytes == 2 );
    if( nBytes == 1 )
        _decompressTensor( reinterpret_cast<uint8_t*>( dst), core, u1, u2, u3, coreDim, blockDim, uStride, scale );
    else
    if( nBytes == 2 )
        _decompressTensor( reinterpret_cast<uint16_t*>(dst), core, u1, u2, u3, coreDim, blockDim, uStride, scale );
    else
        std::cerr << "cpu::decompressTensor: Unsupported byte size!" << std::endl;
}


void _decompressTensor1(      float*    dst,
                        const float*    core,
                        const float*    u1,
                              int       coreDim,
                              int       blockDim,
                              int       uStride  )
{
    const int R    = coreDim;
    const int RR   = R*R;
    const int I    = blockDim;
    const int RI   = R * I;
#pragma omp parallel for
    for( int z = 0; z < R; ++z )                    //   top z: 0..R
    {
        const float* uL = u1;
        for( int y = 0; y < I; ++y )                //   top y: 0..I
        {
            for( int x = 0; x < R; ++x )            //   top x: 0..R
            {
                double sum = 0;
                const float* c = core + x*RR + z*R;
                for( int i = 0; i < R; ++i )        // inner i: 0..R
                {
                    sum += uL[i]*c[i];
                }
                dst[z + y*R + x*RI] = sum;
            }
            uL += uStride;
        }
    }
}


void _decompressTensor2(      float*    dst,
                        const float*    src,
                        const float*    u2,
                              int       coreDim,
                              int       blockDim,
                              int       uStride  )
{
    const int R    = coreDim;
    const int I    = blockDim;
    const int RI   = R * I;

#pragma omp parallel for
    for( int z = 0; z < R; ++z )                    //   top z: 0..R
    {
        const float* uL = u2;
        for( int y = 0; y < I; ++y )                //   top y: 0..I
        {
            for( int x = 0; x < I; ++x )            //   top x: 0..I
            {
                double sum = 0;
                const float* c = src + x*R + z*RI;
                for( int i = 0; i < R; ++i )            // inner i: 0..R
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
void _decompressTensor3(      T*        dst,
                        const float*    src,
                        const float*    u3,
                              int       coreDim,
                              int       blockDim,
                              int       uStride,
                              float     scale   )
{
    const double maxSum = static_cast<double>( massVolVis::Limits<T>::max() );

    const int R    = coreDim;
    const int I    = blockDim;
    const int II   = I * I;
    const int RI   = R * I;

#pragma omp parallel for
    for( int z = 0; z < I; ++z )                    //   top z: 0..I
    {
        const float* uL = u3;
        for( int y = 0; y < I; ++y )                //   top y: 0..I
        {
            for( int x = 0; x < I; ++x )            //   top x: 0..I
            {
                double sum = 0;
                const float* c = src + x*R + z*RI;
                for( int i = 0; i < R; ++i )           // inner i: 0..R
                {
                    sum += uL[i]*c[i];
                }
                dst[y*II + x*I + z] =  hlpFuncs::myClip<double>( sum*scale, 0., maxSum );
            }
            uL += uStride;
        }
    }
}


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
                              byte      nBytes )
{
    assert( nBytes == 1 || nBytes == 2 );

    _decompressTensor1( tmp1, core, u1, coreDim, blockDim, uStride );
    _decompressTensor2( tmp2, tmp1, u2, coreDim, blockDim, uStride );

    if( nBytes == 1 )
        _decompressTensor3( reinterpret_cast<uint8_t*>( dst), tmp2, u3, coreDim, blockDim, uStride, scale );
    else
    if( nBytes == 2 )
        _decompressTensor3( reinterpret_cast<uint16_t*>(dst), tmp2, u3, coreDim, blockDim, uStride, scale );
    else
        std::cerr << "cpu::decompressTensor: Unsupported byte size!" << std::endl;
}


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
                              byte      nBytes )
{
    assert( nBytes == 1 || nBytes == 2 );

    // dequantize core first
    for( int i = 0; i < coreDim*coreDim*coreDim; ++i )
    {
        uint8_t sign = core[i] & 0x80;
        uint8_t abs_value = core[i] & 0x7f;
        tmp2[i] = exp2f( float( abs_value ) * scale ) - 1.0;
        tmp2[i]*= ((sign==0)? -1.f : 1.f );
    }

    _decompressTensor1( tmp1, tmp2, u1, coreDim, blockDim, uStride );
    _decompressTensor2( tmp2, tmp1, u2, coreDim, blockDim, uStride );

    if( nBytes == 1 )
        _decompressTensor3( reinterpret_cast<uint8_t*>( dst), tmp2, u3, coreDim, blockDim, uStride, 1.0 );
    else
    if( nBytes == 2 )
        _decompressTensor3( reinterpret_cast<uint16_t*>(dst), tmp2, u3, coreDim, blockDim, uStride, 1.0 );
    else
        std::cerr << "cpu::decompressTensor: Unsupported byte size!" << std::endl;
}

//-------------------------- Rank reduction functions -------------------------



void _decompressTensor1(      float*    dst,
                        const float*    core,
                        const float*    u1,
                              int       coreDim,
                              int       rank,
                              int       blockDim,
                              int       uStride  )
{
    const int R    = rank;
    const int C    = coreDim;
    const int CC   = C*C;
    const int I    = blockDim;
    const int RI   = R * I;
#pragma omp parallel for
    for( int z = 0; z < R; ++z )                    //   top z: 0..R
    {
        const float* uL = u1;
        for( int y = 0; y < I; ++y )                //   top y: 0..I
        {
            for( int x = 0; x < R; ++x )            //   top x: 0..R
            {
                double sum = 0;
                const float* c = core + x*CC + z*C;
                for( int i = 0; i < R; ++i )        // inner i: 0..R
                {
                    sum += uL[i]*c[i];
                }
                dst[z + y*R + x*RI] = sum;
            }
            uL += uStride;
        }
    }
}


void decompressTensorQuantized2(
                               void*    dst,
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
                              byte      nBytes )
{
    assert( nBytes == 1 || nBytes == 2 );

    // dequantize core first
    for( int i = 0; i < coreDim*coreDim*coreDim; ++i )
    {
        uint8_t sign = core[i] & 0x80;
        uint8_t abs_value = core[i] & 0x7f;
        tmp2[i] = exp2f( float( abs_value ) * scale ) - 1.0;
        tmp2[i]*= ((sign==0)? -1.f : 1.f );
    }

    _decompressTensor1( tmp1, tmp2, u1, coreDim, rank, blockDim, uStride );
    _decompressTensor2( tmp2, tmp1, u2, rank, blockDim, uStride );

    if( nBytes == 1 )
        _decompressTensor3( reinterpret_cast<uint8_t*>( dst), tmp2, u3, rank, blockDim, uStride, 1.0 );
    else
    if( nBytes == 2 )
        _decompressTensor3( reinterpret_cast<uint16_t*>(dst), tmp2, u3, rank, blockDim, uStride, 1.0 );
    else
        std::cerr << "cpu::decompressTensor: Unsupported byte size!" << std::endl;
}

}

