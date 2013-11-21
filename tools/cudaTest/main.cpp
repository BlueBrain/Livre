

#include "cpuDecompressor.h"

#include <msv/util/testing.h>

#define PRINT_TENSOR_TIMINGS
#include "cudaDecompressor.h"

#include <iostream>
#include <vector>
#include <cuda.h>
#include <fstream>


namespace
{
const std::string pathSrt = "/home/maxus/tmp/compressionTst.dat";
}

int main()
{
    std::cout << "Reading input file: " << pathSrt.c_str() << std::endl;

// open input file
    std::ifstream is;
    is.open( pathSrt.c_str(), std::ios_base::in | std::ios::binary );
    if( !is.is_open( ))
    {
        std::cerr << "Can't open file to read: " << pathSrt.c_str() << std::endl;
        return 1;
    }

    byte nBytes     = 0;
    byte coreDim    = 0;
    byte resultDim  = 0;
    float scale     = 0;

    is.read( reinterpret_cast<char*>(&nBytes   ), sizeof(nBytes   ) );
    is.read( reinterpret_cast<char*>(&coreDim  ), sizeof(coreDim  ) );
    is.read( reinterpret_cast<char*>(&resultDim), sizeof(resultDim) );
    is.read( reinterpret_cast<char*>(&scale    ), sizeof(scale    ) );

    int32_t totalSize = sizeof(nBytes) + sizeof(coreDim) + sizeof(resultDim) + sizeof(scale);
    if( is.tellg() != totalSize )
    {
        std::cout << "Can't proceed to the offset: " << totalSize << " to read file: " << pathSrt.c_str() << std::endl;
        is.close();
        return 1;
    }

    std::cout << "  nBytes:    " << (int)nBytes     << std::endl
              << "  coreDim:   " << (int)coreDim    << std::endl
              << "  resultDim: " << (int)resultDim  << std::endl
              << "  scale:     " << (int)scale      << std::endl;

    const int32_t resultSizeTotal = resultDim*resultDim*resultDim;
    const int32_t uStride = coreDim*3;

    std::vector<float> core(  coreDim*coreDim*coreDim );
    std::vector<float> u(     uStride*resultDim );
    std::vector<byte>  res(   resultSizeTotal*nBytes );
    const float* u1 = &u[        0];
    const float* u2 = &u[  coreDim];
    const float* u3 = &u[2*coreDim];


    const size_t cSize = core.size()*sizeof( core[0] );
    const size_t uSize =    u.size()*sizeof(    u[0] );
    const size_t rSize =  res.size()*sizeof(  res[0] );

    is.read( reinterpret_cast<char*>(&core[0]), cSize );// read kernel
    is.read( reinterpret_cast<char*>(   &u[0]), uSize );// read read U1/2/3
    is.read( reinterpret_cast<char*>( &res[0]), rSize );// read result

    totalSize += cSize+uSize+rSize;
    if( is.tellg() != totalSize )
    {
        std::cout << "Can't proceed to the offset: " << totalSize << " to read file: " << pathSrt.c_str() << std::endl;
        is.close();
        return 1;
    }
    is.close();

    std::cout << "Running CPU decompresison... " << std::endl;

// run CPU decompression, compare to result file
    std::vector<byte> cpuRes( resultSizeTotal*nBytes, 0 );
    std::vector<float>  tmp1( resultSizeTotal, 0 );
    std::vector<float>  tmp2( resultSizeTotal, 0 );

    cpu::decompressTensor1( &tmp1[  0], &core[0], coreDim, u1, resultDim, uStride );
    cpu::decompressTensor2( &tmp2[  0], &tmp1[0], coreDim, u2, resultDim, uStride );
    cpu::decompressTensor3( &cpuRes[0], &tmp2[0], coreDim, u3, resultDim, uStride, scale, nBytes );

    std::cout << "Checking CPU decompresison result... " << std::endl;
    testing::testArrays( res, cpuRes );
    std::cout << "CPU decompresison result - ok" << std::endl;

// run GPU decompression, compare to result file
//    std::vector<byte> gpuRes( resultSizeTotal );

    const int coreSize = core.size()*sizeof(core[0]);
    const int gpuResSize = cpuRes.size()*sizeof(cpuRes[0]);
    const int tmp1Size = tmp1.size()*sizeof(tmp1[0]);
    const int tmp2Size = tmp2.size()*sizeof(tmp2[0]);
    void* coreDev = 0;
    void* gpuResDev = 0;
    void* tmp1Dev = 0;
    void* tmp2Dev = 0;
    void*    uDev = 0;
    allocateDeviceArray( &coreDev,   coreSize );
    allocateDeviceArray( &gpuResDev, gpuResSize );
    allocateDeviceArray( &tmp1Dev,   tmp1Size );
    allocateDeviceArray( &tmp2Dev,   tmp2Size );
    allocateDeviceArray( &   uDev,   uSize );
    checkCUDAError( "after allocation 1" );
    assert( coreDev && gpuResSize && tmp1Dev && tmp2Dev && uDev );
    checkCUDAError( "after allocation 2" );

    copyArrayToDevice( coreDev, &core[0], coreSize );
    copyArrayToDevice(    uDev,    &u[0],    uSize );
    checkCUDAError( "after copying to device" );

    std::vector<float> tmp1GPU( resultSizeTotal, 0 );
    std::vector<float> tmp2GPU( resultSizeTotal, 0 );
    std::vector<byte>   gpuRes( gpuResSize,      0 );

    cuDecompTensor( nBytes,
                    static_cast<void*>( gpuResDev ),
                    static_cast<float*>(coreDev),
                    static_cast<float*>(tmp1Dev),
                    static_cast<float*>(tmp2Dev),
                    static_cast<float*>(   uDev),
                    static_cast<float*>(   uDev)+coreDim,
                    static_cast<float*>(   uDev)+coreDim*2,
                    coreDim, resultDim, uStride, scale );

    checkCUDAError( "after decompressing" );

    copyArrayToHost( static_cast<void*>(&tmp1GPU[0]), tmp1Dev, tmp1Size );
    for( size_t i = coreDim*coreDim*resultDim; i < tmp1GPU.size(); ++i )
        tmp1GPU[i] = 0;

    copyArrayToHost( static_cast<void*>(&tmp2GPU[0]), tmp2Dev, tmp2Size );
    for( size_t i = coreDim*resultDim*resultDim; i < tmp2GPU.size(); ++i )
        tmp2GPU[i] = 0;

    copyArrayToHost( static_cast<void*>(&gpuRes[0]), gpuResDev, gpuResSize );

    checkCUDAError( "after copying to host" );


    deleteDeviceArray( &coreDev );
    deleteDeviceArray( &gpuResDev );
    deleteDeviceArray( &tmp1Dev );
    deleteDeviceArray( &tmp2Dev );
    deleteDeviceArray( &   uDev );
    assert( !coreDev && !gpuResDev && !tmp1Dev && !tmp2Dev && !uDev );
    checkCUDAError( "after CUDA memory release" );
    testing::testArrays( tmp1, tmp1GPU );
    std::cout << "GPU decompresison result 1 - ok" << std::endl;
    testing::testArrays( tmp2, tmp2GPU );
    std::cout << "GPU decompresison result 2 - ok" << std::endl;
    testing::testArrays( cpuRes, gpuRes );
    std::cout << "GPU decompresison result 3 - ok" << std::endl;

//    printCUDAInfo();

    return 0;
}








