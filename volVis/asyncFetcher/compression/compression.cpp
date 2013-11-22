
#include "compression.h"

#include "../ramDataElement.h"
#include "../gpuCommands.h"
#include <eq/util/pixelBufferObject.h>
#include <msv/IO/dataHDDIO.h>
#include <msv/IO/dataHDDIOTensor.h>

#include <msv/tree/volumeTreeTensor.h>

#include <msv/util/hlp.h> // cubed

//#undef EQ_USE_CUDA

#include "tensorCPU.h"
#include "tensorCUDA.h"

namespace massVolVis
{

typedef boost::shared_ptr< const VolumeTreeTensor > constVolumeTreeTensorSPtr;


//----------------- DecompressorBase -----------------

DecompressorBase::DecompressorBase( constDataHDDIOSPtr dataIOSPtr, PboSPtr pboSPtr )
    : _dataIOSPtr( dataIOSPtr )
    , _pboSPtr(       pboSPtr )
    , _initialized( false     )
    , _bytesNum( 0 )
{
    if( _dataIOSPtr )
        _bytesNum = _dataIOSPtr->getBytesNum();
}

void* DecompressorBase::_mapWritePbo()
{
    return _pboSPtr ? _pboSPtr->mapWrite() : 0;
}


//----------------- DecompressorNone -----------------

class DecompressorNone : public DecompressorBase
{
public:
    DecompressorNone( constDataHDDIOSPtr dataIOSPtr, PboSPtr pboSPtr )
        : DecompressorBase( dataIOSPtr, pboSPtr ) { _initialized = true; }
    virtual ~DecompressorNone(){}

    virtual bool load( const RAMDataElement* dataEl, GPULoadRequest& request );
};

bool DecompressorNone::load( const RAMDataElement* dataEl, GPULoadRequest& request )
{
    if( !dataEl )
        return false;

    if( void* pboMem = _mapWritePbo( ))
    {
        uint32_t compressedSize = _dataIOSPtr->getBlockSize_( request.treePos );
        LBASSERT( compressedSize <= dataEl->size() );
        memcpy( pboMem, dataEl->data(), compressedSize );
        _pboSPtr->unmap();
        return true;
    }

    LBERROR << "PBO mapping failed" << std::endl;
    return false;
}


//----------------- DecompressorTensorBase -----------------

struct CommonDecompParameters
{
    CommonDecompParameters()
        : u1Offset(0)
        , u2Offset(0)
        , u3Offset(0)
        , coreDim(0)
        , scale(0)
        , blockDim(0)
        , uStride(0)
        {}
    uint32_t u1Offset;
    uint32_t u2Offset;
    uint32_t u3Offset;
    byte     coreDim;
    float    scale;
    uint32_t blockDim;
    uint32_t uStride;
};


class DecompressorTensorBase : public DecompressorBase
{
public:
    DecompressorTensorBase( constDataHDDIOSPtr dataIOSPtr, constVolumeTreeTensorSPtr treeSPtr, PboSPtr pboSPtr );

protected:

    static int _adjustRank( byte coreDim, byte requestRank );
    void _getDecompParameters( uint32_t treePos, CommonDecompParameters& cdp );

    bool _checkCompressedSize( uint32_t compressedSize, uint32_t nodeId ) const;

    const DataHDDIOTensorBase* _dataIOTensorBase; // smart pointer to the _dataIOBase class is handled by DecompressorBase
    std::vector<float> _u123; //!< U1/U2/U3 matrixes for Tensor deconstruction

    uint32_t _blockSize; // in voxels
    uint32_t _compressedBS;
    uint32_t _uncompressedBS;
    constVolumeTreeTensorSPtr _treeSPtr;
};


int DecompressorTensorBase::_adjustRank( byte coreDim, byte requestRank )
{
    return requestRank == 0 ? coreDim : std::min( coreDim, requestRank );
}


bool DecompressorTensorBase::_checkCompressedSize( uint32_t compressedSize, uint32_t nodeId ) const
{
    LBASSERT( compressedSize <= _compressedBS );
    if( compressedSize > 0 )
        return true;

    LBERROR << "Size of compressed data should be > 0; node id: " << nodeId << std::endl;
    return false;
}


void DecompressorTensorBase::_getDecompParameters( uint32_t treePos, CommonDecompParameters& cdp )
{
    const byte  level = VolumeTreeBase::getLevelFromPos( treePos );
    Vec3_i32 bPos = _treeSPtr->getRelativeCoordinates( treePos ).s;

    _dataIOTensorBase->getU123Offsets( cdp.u1Offset, cdp.u2Offset, cdp.u3Offset, level, bPos );
    cdp.coreDim  = _dataIOTensorBase->getNodeRank( treePos );
    cdp.scale    = _treeSPtr->getNodeScale( treePos );
    cdp.blockDim = _dataIOTensorBase->getBlockAndBordersDim();
    cdp.uStride  = _dataIOTensorBase->getUStride();
}


DecompressorTensorBase::DecompressorTensorBase( constDataHDDIOSPtr dataIOSPtr, constVolumeTreeTensorSPtr treeSPtr, PboSPtr pboSPtr )
    : DecompressorBase( dataIOSPtr, pboSPtr )
    , _dataIOTensorBase( dynamic_cast<const DataHDDIOTensorBase*>( dataIOSPtr.get() ))
    , _blockSize(      0 )
    , _compressedBS(   0 )
    , _uncompressedBS( 0 )
    , _treeSPtr( treeSPtr )
{
    LBASSERT( !_initialized );

    if( _bytesNum != 1 && _bytesNum != 2 )
        return;

    if( !pboSPtr || !_dataIOTensorBase || !_dataIOTensorBase->readU123( _u123 ))
        return;

    if( !_treeSPtr )
        return;

    _blockSize = hlpFuncs::cubed( _dataIOTensorBase->getBlockDim());
    _compressedBS = _dataIOTensorBase->getMaxBlockSize();
    _uncompressedBS = _blockSize * _bytesNum;

    LBWARN << "Compressed BS: " << _compressedBS << " Uncompressed BS: " << _uncompressedBS << std::endl;
    if( _compressedBS > _uncompressedBS || _compressedBS < 1 )
        return;

    _initialized = true;
}


//----------------- DecompressorTensorCUDA -----------------

class DecompressorTensorCUDA : public DecompressorTensorBase
{
public:
    // capacity > 1 only if recompression on GPU is required (tensorQuantized)
    DecompressorTensorCUDA( constDataHDDIOSPtr dataIOSPtr, constVolumeTreeTensorSPtr treeSPtr, PboSPtr pboSPtr, uint32_t capacity );
    virtual ~DecompressorTensorCUDA();

protected:

    static void _uploadData( void* dataDev, const byte* data, uint32_t size );

    void* _dataDev; //!< CUDA core storage
    void* _u123Dev; //!< CUDA U1/U2/U3 storage
    void* _tmp1Dev; //!< CUDA tmp1 storage
    void* _tmp2Dev; //!< CUDA tmp1 storage

    uint32_t _capacity;
};


void DecompressorTensorCUDA::_uploadData( void* dataDev, const byte* data, uint32_t size )
{
    checkCUDAError( "before data to device uploading" );
    copyArrayToDevice( dataDev, data, size );
    checkCUDAError( "after data to device uploading" );
}


DecompressorTensorCUDA::DecompressorTensorCUDA( constDataHDDIOSPtr dataIOSPtr, constVolumeTreeTensorSPtr treeSPtr, PboSPtr pboSPtr, uint32_t capacity )
    : DecompressorTensorBase( dataIOSPtr, treeSPtr, pboSPtr )
    , _dataDev( 0 )
    , _u123Dev( 0 )
    , _tmp1Dev( 0 )
    , _tmp2Dev( 0 )
    , _capacity( capacity )
{
    if( !_initialized )
        return;
    _initialized = false;

    if( capacity < 1 )
        return;

    checkCUDAError( "before CUDA BPO registration" );
    registerGLBufferObject( _pboSPtr->getID() );

    _dataIOSPtr->getBytesNum();
    allocateDeviceArray( &_dataDev, _compressedBS*_capacity );
    allocateDeviceArray( &_tmp1Dev, _blockSize*sizeof(float) );
    allocateDeviceArray( &_tmp2Dev, _blockSize*sizeof(float) );
    allocateDeviceArray( &_u123Dev, _u123.size()*sizeof(_u123[0]) );

    checkCUDAError( "after array allocation" );
    copyArrayToDevice( _u123Dev, &_u123[0], _u123.size()*sizeof(_u123[0]) );
    checkCUDAError( "after u123 upload" );

    _initialized = true;
}


DecompressorTensorCUDA::~DecompressorTensorCUDA()
{
    unregisterGLBufferObject( _pboSPtr->getID() );

    checkCUDAError( "Before delete CUDA storage" );
    deleteDeviceArray( &_dataDev );
    deleteDeviceArray( &_u123Dev );
    deleteDeviceArray( &_tmp1Dev );
    deleteDeviceArray( &_tmp2Dev );
    checkCUDAError( "After delete CUDA storage" );
}


//----------------- DecompressorTensorCUDA -----------------

class DecompressorTensorBasicCUDA : public DecompressorTensorCUDA
{
public:
    DecompressorTensorBasicCUDA( constDataHDDIOSPtr dataIOSPtr, constVolumeTreeTensorSPtr treeSPtr, PboSPtr pboSPtr )
        : DecompressorTensorCUDA( dataIOSPtr, treeSPtr, pboSPtr, 1 ){}

        virtual ~DecompressorTensorBasicCUDA(){}

    bool load( const RAMDataElement* dataEl, GPULoadRequest& request );
};


bool DecompressorTensorBasicCUDA::load( const RAMDataElement* dataEl, GPULoadRequest& request )
{
    if( !dataEl )
        return false;

    const uint32_t compressedSize = _dataIOSPtr->getBlockSize_( request.treePos );
    if( !_checkCompressedSize( compressedSize, request.nodeId ))
        return false;

    CommonDecompParameters cdp;
    _getDecompParameters( request.treePos, cdp );

    void* dataDev = reinterpret_cast<uint8_t*>(_dataDev);

    _uploadData( dataDev, dataEl->data(), compressedSize );

    cudaDecompTensor( _pboSPtr->getID(),
                    static_cast<float*>( _tmp1Dev ),
                    static_cast<float*>( _tmp2Dev ),
                    static_cast<float*>( dataDev  ),
                    static_cast<float*>( _u123Dev ) + cdp.u1Offset,
                    static_cast<float*>( _u123Dev ) + cdp.u2Offset,
                    static_cast<float*>( _u123Dev ) + cdp.u3Offset,
                    cdp.coreDim, cdp.blockDim, cdp.uStride, cdp.scale, _bytesNum );

    checkCUDAError( "after data decompression" );
    return true;
}


//----------------- DecompressorTensorQuantizedCUDA -----------------

class DecompressorTensorQuantizedCUDA : public DecompressorTensorCUDA
{
public:
    DecompressorTensorQuantizedCUDA( constDataHDDIOSPtr dataIOSPtr, constVolumeTreeTensorSPtr treeSPtr, PboSPtr pboSPtr, uint32_t capacity )
        : DecompressorTensorCUDA( dataIOSPtr, treeSPtr, pboSPtr, capacity ){}

    virtual ~DecompressorTensorQuantizedCUDA(){}

    virtual bool rankDependant() const { return true; }

    virtual bool supportsFastReloading() const { return true; }

    bool load( const RAMDataElement* dataEl, GPULoadRequest& request );
};


bool DecompressorTensorQuantizedCUDA::load( const RAMDataElement* dataEl, GPULoadRequest& request )
{
    if( !dataEl )
        return false;

    const uint32_t compressedSize = _dataIOSPtr->getBlockSize_( request.treePos );
    if( !_checkCompressedSize( compressedSize, request.nodeId ))
        return false;

    CommonDecompParameters cdp;
    _getDecompParameters( request.treePos, cdp );
    const int rank = _adjustRank( cdp.coreDim, request.rank );

    void* dataDev = reinterpret_cast<uint8_t*>(_dataDev) + _compressedBS*request.posOnGPU;

    if( !request.reload )
        _uploadData( dataDev, dataEl->data(), compressedSize );

    cudaDecompTensorQuantized( _pboSPtr->getID(),
                    static_cast<float*>( _tmp1Dev ),
                    static_cast<float*>( _tmp2Dev ),
                    static_cast<uint8_t*>( dataDev  ),
                    static_cast<float*>( _u123Dev ) + cdp.u1Offset,
                    static_cast<float*>( _u123Dev ) + cdp.u2Offset,
                    static_cast<float*>( _u123Dev ) + cdp.u3Offset,
                    cdp.coreDim, rank, cdp.blockDim, cdp.uStride, cdp.scale, _bytesNum );

    checkCUDAError( "after data decompression" );
    return true;
}


//----------------- DecompressorTensorCPU -----------------

class DecompressorTensorCPU : public DecompressorTensorBase
{
public:
    DecompressorTensorCPU( constDataHDDIOSPtr dataIOSPtr, constVolumeTreeTensorSPtr treeSPtr, PboSPtr pboSPtr )
        : DecompressorTensorBase( dataIOSPtr, treeSPtr, pboSPtr ){}

        virtual ~DecompressorTensorCPU(){}

    bool load( const RAMDataElement* dataEl, GPULoadRequest& request );

private:
    std::vector<float> _tmp1;
    std::vector<float> _tmp2;
};


bool DecompressorTensorCPU::load( const RAMDataElement* dataEl, GPULoadRequest& request )
{
    if( !dataEl )
        return false;

    const uint32_t compressedSize = _dataIOSPtr->getBlockSize_( request.treePos );
    if( !_checkCompressedSize( compressedSize, request.nodeId ))
        return false;

    CommonDecompParameters cdp;
    _getDecompParameters( request.treePos, cdp );

    void* pboMem = 0;
    if( !(pboMem = _mapWritePbo( )))
    {
        LBERROR << "PBO mapping failed" << std::endl;
        return false;
    }

    _tmp1.resize( hlpFuncs::cubed( cdp.blockDim ));
    _tmp2.resize( hlpFuncs::cubed( cdp.blockDim ));

    cpu::decompressTensorBasic(
                    pboMem,
                    &_tmp1[0],
                    &_tmp2[0],
                    reinterpret_cast<const float*>(dataEl->data()),
                    &_u123[ cdp.u1Offset ],
                    &_u123[ cdp.u2Offset ],
                    &_u123[ cdp.u3Offset ],
                    cdp.coreDim, cdp.blockDim, cdp.uStride, cdp.scale, _bytesNum );
    _pboSPtr->unmap();

    return true;
}


//----------------- DecompressorTensorQuantizedCPU -----------------

class DecompressorTensorQuantizedCPU : public DecompressorTensorBase
{
public:
    DecompressorTensorQuantizedCPU( constDataHDDIOSPtr dataIOSPtr, constVolumeTreeTensorSPtr treeSPtr, PboSPtr pboSPtr )
        : DecompressorTensorBase( dataIOSPtr, treeSPtr, pboSPtr ){}

    virtual ~DecompressorTensorQuantizedCPU(){}

    virtual bool rankDependant() const { return true; }

    bool load( const RAMDataElement* dataEl, GPULoadRequest& request );

private:
    std::vector<float> _tmp1;
    std::vector<float> _tmp2;
};


bool DecompressorTensorQuantizedCPU::load( const RAMDataElement* dataEl, GPULoadRequest& request )
{
    if( !dataEl )
        return false;

    const uint32_t compressedSize = _dataIOSPtr->getBlockSize_( request.treePos );
    if( !_checkCompressedSize( compressedSize, request.nodeId ))
        return false;

    CommonDecompParameters cdp;
    _getDecompParameters( request.treePos, cdp );

    void* pboMem = 0;
    if( !(pboMem = _mapWritePbo( )))
    {
        LBERROR << "PBO mapping failed" << std::endl;
        return false;
    }

    const int rank = _adjustRank( cdp.coreDim, request.rank );

    _tmp1.resize( hlpFuncs::cubed( cdp.blockDim ));
    _tmp2.resize( hlpFuncs::cubed( cdp.blockDim ));

    cpu::decompressTensorQuantized2(
                    pboMem,
                    &_tmp1[0],
                    &_tmp2[0],
                    dataEl->data(),
                    &_u123[ cdp.u1Offset ],
                    &_u123[ cdp.u2Offset ],
                    &_u123[ cdp.u3Offset ],
                    cdp.coreDim, rank, cdp.blockDim, cdp.uStride, cdp.scale, _bytesNum );
    _pboSPtr->unmap();

    return true;
}


//----------------- Decompressor selection -----------------

namespace
{
DecompressorSPtr _select( constDataHDDIOSPtr dataIOSPtr, constVolumeTreeBaseSPtr volumeTreeSPtr, PboSPtr pboSPtr, uint32_t capacity )
{
    if( !dataIOSPtr || !pboSPtr )
    {
        LBERROR << "Can't intialize decompressor" << std::endl;
        return DecompressorSPtr();
    }

    constVolumeTreeTensorSPtr volumeTreeTensorSPtr = boost::dynamic_pointer_cast<const VolumeTreeTensor>( volumeTreeSPtr );
    if( !volumeTreeTensorSPtr )
        return DecompressorSPtr( new DecompressorNone( dataIOSPtr, pboSPtr ));

    const bool isTensorQuantized = dynamic_cast< const DataHDDIOTensorQuantized* >( dataIOSPtr.get());

#ifdef EQ_USE_CUDA
    LBWARN << "CUDA is enabled" << std::endl;
    if( isTensorQuantized )
        return DecompressorSPtr( new DecompressorTensorQuantizedCUDA( dataIOSPtr, volumeTreeTensorSPtr, pboSPtr, capacity ));
    else
        return DecompressorSPtr( new DecompressorTensorBasicCUDA( dataIOSPtr, volumeTreeTensorSPtr, pboSPtr ));
#endif
    LBWARN << "CUDA is NOT enabled" << std::endl;
    if( isTensorQuantized )
        return DecompressorSPtr( new DecompressorTensorQuantizedCPU( dataIOSPtr, volumeTreeTensorSPtr, pboSPtr ));
    else
        return DecompressorSPtr( new DecompressorTensorCPU( dataIOSPtr, volumeTreeTensorSPtr, pboSPtr ));
}
}

DecompressorSPtr Decompressors::select( constDataHDDIOSPtr dataIOSPtr, constVolumeTreeBaseSPtr volumeTreeSPtr, PboSPtr pboSPtr, uint32_t capacity )
{
    DecompressorSPtr decomp = _select( dataIOSPtr, volumeTreeSPtr, pboSPtr, capacity );
    if( decomp && !decomp->isIntialized() )
        return DecompressorSPtr();

    return decomp;
}

}
