/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "dataHDDIOTensor.h"

#include <msv/tree/volumeTreeTensor.h>

#include <msv/util/fileIO.h>

#include <msv/util/str.h>  // toString
#include <msv/util/hlp.h>  // fileSize
#include <msv/util/debug.h>

#include <assert.h>
#include <math.h>  // log2f
#include <cstring> // memcpy
#include <cstdlib> // abort

namespace massVolVis
{


DataHDDIOTensorBase::DataHDDIOTensorBase( const VolumeFileInfo& fileInfo )
    : DataHDDIO( fileInfo )
    , _baseName( "/core_" )
{
    uint32_t treeSize = VolumeTreeBase::getNumberOfNodes( getSourceDims(), getBlockDim() );
    _ranks    = byteVecSPtr(   new byteVec(   treeSize ));
    _scales   = floatVecSPtr(  new floatVec(  treeSize ));
    _nodesIds = NodeIdVecSPtr( new NodeIdVec( treeSize ));
}


bool DataHDDIOTensorBase::_initTree()
{
    const bool result = _readIdsRanksScales();

    _tree = _createTree();
    _tree->loadHistograms( getHistorgamFileName() );
    return result;
}


VolumeTreeBaseSPtr DataHDDIOTensorBase::_createTree()
{
    return VolumeTreeBaseSPtr(
        new VolumeTreeTensor( getSourceDims(), getBlockDim(), _nodesIds, _ranks, _scales ));
}


byte DataHDDIOTensorBase::getNodeRank( const uint32_t pos ) const
{
    if( pos >= _ranks->size() )
    {
        LOG_ERROR << "Invalid id: " << pos << std::endl;
        return 0;
    }
    return (*_ranks)[ pos ];
}


Vec2_ui32 DataHDDIOTensorBase::getFullU123SrcDims( byte& depth ) const
{
    const uint32_t bd = getBlockDim();
    Vec2_ui32 dims = getU123SrcDims();
    std::cout << "getFullU123SrcDims: " << dims << std::endl;

    assert( dims.h % bd == 0 );
    uint32_t d = dims.h/bd;
    assert( d != 0 && (d & (~d+1)) == d ); // power of 2

    dims.h = dims.h*2 - bd;

    // first least significant bit (log_2() + 1)
    static const byte multiplyDeBruijnBitPosition[32] = {
        0+1, 1+1, 28+1, 2+1, 29+1, 14+1, 24+1, 3+1, 30+1, 22+1, 20+1, 15+1, 25+1, 17+1, 4+1, 8+1,
        31+1, 27+1, 13+1, 23+1, 21+1, 19+1, 16+1, 7+1, 26+1, 12+1, 18+1, 6+1, 11+1, 5+1, 10+1, 9+1 };

    depth = multiplyDeBruijnBitPosition[((uint32_t)((d & -d) * 0x077CB531U)) >> 27];

//    std::cout << "bs: " << (int)bd << "depth: " << (int)depth << " Dims: " << dims << std::endl;

    return dims;
}


Vec2_ui32 DataHDDIOTensorBase::getFullU123Dims() const
{
    byte depth; return getFullU123Dims( depth );
}


/**
 *  Name should contain directory of the octree
 */
void DataHDDIOTensorBase::_getFileNameAndOffset( const uint32_t     id,
                                                   std::string& name, uint32_t&  offset )
{
    name.append( _baseName );
    name.append( strUtil::toString( id ));
    name.append( ".raw" );

    offset = 0;
}


std::string DataHDDIOTensorBase::getU123FileName( byte u ) const
{
    assert( u >= 1 && u <= 3 );
    return getDataFileDir() + "/u" + strUtil::toString( uint32_t(u) ) + ".raw";
}


std::string DataHDDIOTensorBase::getRanksFileName() const
{
    return  getDataFileDir() + "/ranks.raw";
}


bool DataHDDIOTensorBase::read( const uint32_t id, void* dst )
{
    assert( isBlockSizeValid() );
    assert( id != 0 );

    std::string fName = getDataFileDir();
    uint32_t    offset;
    _getFileNameAndOffset( id, fName, offset );

    const uint32_t fSize = util::fileSize( fName );
    assert( fSize <= DataHDDIO::getBlockSize_() );

    util::InFile inFile;

    if( inFile.open( fName, std::ios::binary, true ) &&
        inFile.read( offset, fSize, dst ))
        return true;

    return false;
}


//============================ DataHDDIOTensorBasic functions ============================


DataHDDIOTensorBasic::DataHDDIOTensorBasic( const VolumeFileInfo& fileInfo, bool initTree )
    : DataHDDIOTensorBase( fileInfo )
{
    if( initTree )
        _initTree();
}


uint32_t DataHDDIOTensorBasic::getBlockSize_( const uint32_t pos ) const
{
    const uint32_t r = getNodeRank( pos );
    return r*r*r*sizeof(float);
}


uint32_t DataHDDIOTensorBasic::getU1Offset( byte level ) const
{
    const byte  borderDim = getBorderDim();
    const Vec2_ui16& dims = getU123Dims();
          uint32_t     bd = getBlockDim();

    uint32_t h = 0;
    for( byte d = 0; d < level; ++d )
    {
        h += borderDim + bd;
        bd *= 2;
    }
    return h * dims.w*3;
}


Vec2_ui32 DataHDDIOTensorBasic::getFullU123Dims( byte& depth ) const
{
    Vec2_ui32 dims = getFullU123SrcDims( depth );

    dims.w  = getU123Dims().w;
    dims.h += getBorderDim()*(depth+1);

    return dims;
}


bool DataHDDIOTensorBasic::readU123( std::vector<float>& dst ) const
{
    byte depth = 0;
    const Vec2_ui32 dimsSrc     = getU123SrcDims();
    const Vec2_ui32 dimsSrcFull = getFullU123SrcDims( depth );
    assert( dimsSrc.w == dimsSrcFull.w );
    assert( depth > 0 );
    assert( (getBlockDim() << (depth-1)) == dimsSrc.h );

    std::vector<float> tmp( dimsSrcFull.getAreaSize()*3, 0 );

    assert( dimsSrc.getAreaSize() > 0 );
    assert( sizeof(float) == 4 );

// Read original matrixes into an array that has reserved space for mip-map
    float* d = &tmp[0];
    for( byte u = 1; u <= 3; ++u )
    {
        const std::string fName = getU123FileName( u );
        std::cout << "fs: " << util::fileSize( fName ) << " dimsSrc: " << dimsSrc << std::endl;
        assert( util::fileSize( fName ) == dimsSrc.getAreaSize()*sizeof(tmp[0]) );

        util::InFile inFile;
        if( !inFile.open( fName, std::ios::binary, true ) ||
            !inFile.read( 0, 0, 0 ))
            return false;

        // TODO : why not just read the entire file at once?
        const uint32_t bytesToRead = dimsSrc.h*sizeof(*d);
        for( uint32_t column = 0; column < dimsSrc.w; ++column )
        {
            if( !inFile.read( bytesToRead, d ))
                break;
            d += dimsSrcFull.h;
        }
    }

// Create scaled versions
    for( uint32_t column = 0; column < dimsSrc.w*3; ++column )
    {
        uint32_t sI = column*dimsSrcFull.h;
        uint32_t dI = sI + dimsSrc.h;
        while( dI-sI > 64 )
        {
            assert( dI   < tmp.size() );
            assert( sI+1 < tmp.size() );
            tmp[dI] = (tmp[sI] + tmp[sI+1]) / 2.0;
            ++dI;
            sI +=2;
        }
    }

// shift columns per level
    const Vec2_ui32 dstDims = getU123Dims();
    assert( dstDims.w > 0 && dstDims.h > 0 );

    // compute offsets for levels
    std::vector<uint32_t> offsets;
    if( isAttributeSet( U_OFFSETS ))
    {
        LOG_INFO << "Data offsets: " << std::endl;
        std::stringstream ss( getUOffsets() );
        while( !ss.eof() )
        {
            uint32_t offset = 0;
            ss >> offset;

            assert( offset < dimsSrcFull.w );
            if( offset > dimsSrcFull.w-1 )
            {
                LOG_ERROR << "Incorrect UOffset: " << offset << " USrcSize: " << dimsSrcFull.w << std::endl;
                offset = dimsSrcFull.w-1;
            }
            offsets.push_back( offset );
            LOG_INFO << "  level " << offsets.size()-1 << ": " << offsets.back() << std::endl;

            if( offsets.size() > 10 )
            {
                LOG_ERROR << "Too many U offsets!" << std::endl;
                break;
            }
        }
    }

// 1) Transpose (data originally is column-wise and we need raw-wise order)
// 2) add borders
// 3) drop columns
// 4) reorder levels
    dst.resize( DataHDDIOTensorBase::getFullU123Dims().getAreaSize()*3 );
    memset( &dst[0], 0, dst.size()*sizeof(dst[0]) );

    const uint32_t borderOffset = getBorderDim()*dstDims.w*3;
    std::cout << " borderOffset:  " << borderOffset << std::endl;

    uint32_t levelHeight = dimsSrc.h;
    uint32_t srcU1 = 0;
    uint32_t srcU2 = srcU1+dimsSrcFull.getAreaSize();
    uint32_t srcU3 = srcU2+dimsSrcFull.getAreaSize();
    std::cout << "dimsSrcFull.getAreaSize()" << dimsSrcFull.getAreaSize() << std::endl;
    for( byte level = depth; level > 0 ; --level )
    {
        uint32_t columnShift = level > offsets.size() ? 0 : offsets[level-1];
        size_t dstU1 = getU1Offset( level-1 ) + borderOffset;
        size_t dstU2 = dstU1 + dstDims.w;
        size_t dstU3 = dstU2 + dstDims.w;
        std::cout << " level: " << (int)level << " dstU1:  " << dstU1 << std::endl;
        for( uint32_t row = 0; row < levelHeight; ++row )
        {
            const uint32_t maxCol = hlpFuncs::myMin( dstDims.w, dimsSrcFull.w-columnShift );
            for( uint32_t col = 0; col < maxCol; ++col )
            {
                const uint32_t srcShift = (col+columnShift)*dimsSrcFull.h + row;
                assert( srcU3 + srcShift < tmp.size() );
                assert( dstU3 + col      < dst.size() );
                dst[ dstU1 + col ] = tmp[ srcU1 + srcShift ];
                dst[ dstU2 + col ] = tmp[ srcU2 + srcShift ];
                dst[ dstU3 + col ] = tmp[ srcU3 + srcShift ];
            }
            dstU1 += dstDims.w*3;
            dstU2 += dstDims.w*3;
            dstU3 += dstDims.w*3;
        }
        srcU1 += levelHeight;
        srcU2 += levelHeight;
        srcU3 += levelHeight;
        levelHeight /= 2;
    }
    return true;
}


bool DataHDDIOTensorBasic::_readIdsRanksScales()
{
    assert( _nodesIds && _ranks && _scales );

    const size_t size = _nodesIds->size();

    if( _ranks->size()  != size ||
        _scales->size() != size )
    {
        LOG_ERROR << "Size of _ranks or _scales is incorect " << std::endl;
        return false;
    }

    memset( &((*_nodesIds)[ 0]), 0, size*sizeof((*_nodesIds)[0]));
    memset( &((*_ranks)[    0]), 0, size*sizeof((*_ranks)[   0]));
    memset( &((*_scales)[   0]), 0, size*sizeof((*_scales)[  0]));

    std::vector<NodeId> tNodes(  size );
    std::vector<byte>   tRanks(  size );
    std::vector<float>  tScales( size );
    memset( &tNodes[ 0], 0, tNodes.size() *sizeof(tNodes[ 0]));
    memset( &tRanks[ 0], 0, tRanks.size() *sizeof(tRanks[ 0]));
    memset( &tScales[0], 0, tScales.size()*sizeof(tScales[0]));

    std::string fName = getRanksFileName();

// Read ranks, build ids
    std::vector<double> data( size );
    const uint32_t bytesToRead = data.size()*sizeof(data[0]);

    util::InFile inFile;
    if( !inFile.open( fName, std::ios::binary, true ) ||
        !inFile.read( bytesToRead, bytesToRead, &data[0] ))
        return false;

    byte maxRank = 0;
    for( size_t i = 0; i < data.size(); ++i )
    {
        int32_t rank = static_cast<int32_t>( data[i]+0.5 );
        if( rank < 0 || rank > 255 )
        {
            LOG_ERROR << "Incorrect rank? (" << rank << ")" << std::endl;
            break;
        }
        if( rank > 0 )
        {
            tNodes[i] = i+1;
            tRanks[i] = rank;
            if( maxRank < rank )
                maxRank = rank;
        }
    }
    setMaxRankDim( maxRank );

// Read scales
    if( !inFile.read( bytesToRead*4, bytesToRead, &data[0] ))
        return false;

    for( size_t i = 0; i < data.size(); ++i )
    {
        if( tRanks[i] > 0 )
            tScales[i] = data[i];
        else // sanity check
            if( data[i] > 0.5 )
            {
                LOG_ERROR << "Incorrect scale? Pos: " << i << " scale: " << data[i] << std::endl;
                break;
            }
    }

// Fix wrong order from TENSOR iteration
    uint32_t side    = 1;
    uint32_t current = 0;
    uint32_t start   = 0;
    std::cout << "size: " << size << std::endl;
    for( size_t i = 0; i < size; ++i )
    {
        if( current == side*side*side )
        {
            start   = VolumeTreeBase::getChild( start );
            side   *= 2;
            current = 0;
        }
        Vec3_ui32 p = VolumeTreeBase::getIndexPosition( current++ );
        uint32_t pos = start + (p.z*side+p.y)*side+p.x;
//        std::cout << "i: " << i <<  " pos: " << pos << " current: " << (current-1) << " side: " << side << std::endl;
        assert( pos < size );

        (*_nodesIds)[i] = tNodes[ pos];
        (*_ranks)[   i] = tRanks[ pos];
        (*_scales)[  i] = tScales[pos];
    }

#if 0 // print first elements of rank array
    for( size_t i = 0; i < 1+8+64; ++i )
    {
        std::cout << "i: " << i << " id: " << (int)nodes[i].id << " rank: " << (int)ranks[i] << " scale: " << scales[i] << std::endl;
    }
#endif
    return true;
}


//============================ DataHDDIOTensorQuantized functions ============================


DataHDDIOTensorQuantized::DataHDDIOTensorQuantized( const VolumeFileInfo& fileInfo, bool initTree )
    : DataHDDIOTensorBase( fileInfo )
{
    if( initTree )
        _initTree();
}


uint32_t DataHDDIOTensorQuantized::getBlockSize_( const uint32_t pos ) const
{
    const uint32_t r = getNodeRank( pos );
    return r*r*r*sizeof(byte);
}


uint32_t DataHDDIOTensorQuantized::getU1Offset( byte level ) const
{
    const byte  borderDim = getBorderDim();
    const Vec2_ui16& dims = getU123Dims();
          uint32_t     bd = getBlockDim();

    uint32_t h = 0;
    for( byte d = 0; d < level; ++d )
    {
        h += borderDim + bd + ((1<<d)-1)*borderDim*2;
        bd *= 2;
    }
    return h * dims.w*3;
}


Vec2_ui32 DataHDDIOTensorQuantized::getFullU123Dims( byte& depth ) const
{
    const byte borderDim = getBorderDim();
    const uint32_t bd = getBlockDim();
    Vec2_ui32 dims = getU123SrcDims();
    std::cout << "getFullU123SrcDims: " << dims << std::endl;
    uint32_t dH = dims.h;
    depth = 1;
    while( dH >= bd*2 )
    {
        dH /= 2;
        dims.h += dH + ((1<<depth)-1)*borderDim*2; // blocks + inner borders
        ++depth;
        assert( depth < 20 ); // some large number of levels
    }
    std::cout << "dH: " << dH << " bd:" << bd << std::endl;
    assert( dH == bd );

    dims.w  = getU123Dims().w;
    dims.h += borderDim*(depth+1); // outer borders

    return dims;
}


bool DataHDDIOTensorQuantized::readU123( std::vector<float>& dst ) const
{
    byte depth = 0;
    const Vec2_ui32 dimsFull = getFullU123Dims( depth );
    assert( depth > 0 );
    assert( getU123Dims().w == dimsFull.w );
//    assert( (getBlockDim() << (depth-1)) == dimsSrc.h );

    std::vector<float> tmp( dimsFull.getAreaSize()*3, 0 );

    assert( getU123Dims().getAreaSize() > 0 );
    assert( sizeof(float) == 4 );

// Read original matrixes into an array that has reserved space for mip-map
    float* d = &tmp[0];
    for( byte u = 1; u <= 3; ++u )
    {
        const std::string fName = getU123FileName( u );
        const uint32_t fileSize = util::fileSize( fName );
        std::cout << "fn: " << fName << " fs: " << fileSize << " dimsFull: " << dimsFull << std::endl;
        assert( fileSize >= dimsFull.getAreaSize()*sizeof(tmp[0]) );

        util::InFile inFile;
        if( !inFile.open( fName, std::ios::binary, true ))
            return false;

        const uint32_t filePitch = fileSize / dimsFull.w;
        const uint32_t bytesToRead = dimsFull.h*sizeof(*d);
        for( uint32_t column = 0; column < dimsFull.w; ++column )
        {
            if( !inFile.read( filePitch*column, bytesToRead, d ))
                break;
            d += dimsFull.h;
        }
    }

// 1) Transpose (data originally is column-wise and we need raw-wise order)

    dst.resize( dimsFull.getAreaSize()*3 );
    memset( &dst[0], 0, dst.size()*sizeof(dst[0]) );

    for( uint32_t row = 0; row < dimsFull.h; ++row )
        for( uint32_t column = 0; column < dimsFull.w*3; ++column )
        {
            dst[row*dimsFull.w*3 + column] = tmp[column*dimsFull.h + row];
        }

#if 0
    for( int r = 1; r <= 3; ++r )
    {
        for( int c = 0; c < 32; ++c )
        {
            std::cout << dst[ r*dimsFull.w*3 + c * 3 ] << " ";
        }
        std::cout << std::endl;
    }
#endif

    return true;
}


bool DataHDDIOTensorQuantized::_readIdsRanksScales( )
{
    assert( _nodesIds && _ranks && _scales );

    const size_t size = _nodesIds->size();

    if( _ranks->size()  != size ||
        _scales->size() != size )
    {
        LOG_ERROR << "Size of _ranks or _scales is incorect " << std::endl;
        return false;
    }

    memset( &((*_nodesIds)[ 0]), 0, size*sizeof((*_nodesIds)[0]));
    memset( &((*_ranks)[    0]), 0, size*sizeof((*_ranks)[   0]));
    memset( &((*_scales)[   0]), 0, size*sizeof((*_scales)[  0]));

    std::vector<NodeId> tNodes(  size );
    std::vector<byte>   tRanks(  size );
    std::vector<float>  tScales( size );
    memset( &tNodes[ 0], 0, tNodes.size() *sizeof(tNodes[ 0]));
    memset( &tRanks[ 0], 0, tRanks.size() *sizeof(tRanks[ 0]));
    memset( &tScales[0], 0, tScales.size()*sizeof(tScales[0]));

    std::string fName = getRanksFileName();

// Read ranks, build ids
    std::vector<double> data( size );
    const uint32_t bytesToRead = data.size()*sizeof(data[0]);

    util::InFile inFile;
    if( !inFile.open( fName, std::ios::binary, true ) ||
        !inFile.read( bytesToRead, bytesToRead, &data[0] ))
        return false;

    byte maxRank = 0;
    for( size_t i = 0; i < data.size(); ++i )
    {
        int32_t rank = static_cast<int32_t>( data[i]+0.5 );
        if( rank < 0 || rank > 255 )
        {
            LOG_ERROR << "Incorrect rank? (" << rank << ")" << std::endl;
            break;
        }
        if( rank > 0 )
        {
            tNodes[i] = i+1;
            tRanks[i] = rank;
            if( maxRank < rank )
                maxRank = rank;
        }
    }
    setMaxRankDim( maxRank );

// Read scales
    if( !inFile.read( bytesToRead*4, bytesToRead, &data[0] ))
        return false;

    for( size_t i = 0; i < data.size(); ++i )
    {
        if( tRanks[i] > 0 )
            tScales[i] = log2f( 1.0 + data[i] ) / 127.f;
    }

// Fix wrong order from TENSOR iteration
    uint32_t side    = 1;
    uint32_t current = 0;
    uint32_t start   = 0;
    std::cout << "size: " << size << std::endl;
    for( size_t i = 0; i < size; ++i )
    {
        if( current == side*side*side )
        {
            start   = VolumeTreeBase::getChild( start );
            side   *= 2;
            current = 0;
        }
        Vec3_ui32 p = VolumeTreeBase::getIndexPosition( current++ );
        uint32_t pos = start + (p.z*side+p.y)*side+p.x;
//        std::cout << "i: " << i <<  " pos: " << pos << " current: " << (current-1) << " side: " << side << std::endl;
        assert( pos < size );

        (*_nodesIds)[i] = tNodes[ pos];
        (*_ranks)[   i] = tRanks[ pos];
        (*_scales)[  i] = tScales[pos];
    }

#if 0 // print first elements of rank array
    for( size_t i = 0; i < 1+8+64; ++i )
    {
        std::cout << "i: " << i << " id: " << (int)nodes[i].id << " rank: " << (int)ranks[i] << " scale: " << scales[i] << std::endl;
    }
#endif
    return true;
}


//============================ DataHDDIOTensorQuantized2G functions ============================


DataHDDIOTensorQuantized2G::DataHDDIOTensorQuantized2G( const VolumeFileInfo& fileInfo, bool initTree )
    : DataHDDIOTensorQuantized( fileInfo, false )
{
    if( initTree )
        _initTree();
}


bool DataHDDIOTensorQuantized2G::_readIdsRanksScales( )
{
    assert( _nodesIds && _ranks && _scales );

    const size_t size = _nodesIds->size();

    if( _ranks->size()  != size ||
        _scales->size() != size )
    {
        LOG_ERROR << "Size of _ranks or _scales is incorect " << std::endl;
        return false;
    }

    memset( &((*_nodesIds)[ 0]), 0, size*sizeof((*_nodesIds)[0]));
    memset( &((*_ranks)[    0]), 0, size*sizeof((*_ranks)[   0]));
    memset( &((*_scales)[   0]), 0, size*sizeof((*_scales)[  0]));

    std::string fName = getRanksFileName();

// Read ranks, build ids
    std::vector<double> data( size );
    const uint32_t bytesToRead = data.size()*sizeof(data[0]);

    util::InFile inFile;
    if( !inFile.open( fName, std::ios::binary, true ) ||
        !inFile.read( bytesToRead, bytesToRead, &data[0] ))
        return false;

    byte     maxRank   = 0;
    uint32_t currentId = 0;
    for( size_t i = 0; i < data.size(); ++i )
    {
        int32_t rank = static_cast<int32_t>( data[i]+0.5 );
        if( rank < 0 || rank > 255 )
        {
            LOG_ERROR << "Incorrect rank? (" << rank << ")" << std::endl;
            break;
        }
        if( rank > 0 )
        {
            currentId++;
            (*_nodesIds)[i] = currentId;
            (*_ranks)[   i] = rank;
            if( maxRank < rank )
                maxRank = rank;
        }
    }
    setMaxRankDim( maxRank );

    // check that all non zero ranks have the same size
    for( size_t i = 0; i < size; ++i )
        if( (*_ranks)[i] != 0 && (*_ranks)[i] != maxRank )
        {
            LOG_ERROR << "One of the non-zero ranks is not equal to maxRank!" << std::endl;
            abort();
        }

    // Read scales
    if( !inFile.read( bytesToRead*4, bytesToRead, &data[0] ))
        return false;

    for( size_t i = 0; i < data.size(); ++i )
    {
        if( (*_ranks)[i] > 0 )
            (*_scales)[i] = log2f( 1.0 + data[i] ) / 127.f;
    }

#if 0 // print first elements of rank array
    for( size_t i = 0; i < 1+8+64; ++i )
    {
        std::cout << "i: " << i << " id: " << (int)nodes[i].id << " rank: " << (int)ranks[i] << " scale: " << scales[i] << std::endl;
    }
#endif
    return true;
}


namespace
{
/**
 *  Returns a ^2 number of blocks that fits in one file, given the file size is 2 GB.
 *  id of the octree can be shifted left by that number to get the file number.
 */
uint32_t _pow2NumberOfBlocks( uint32_t size )
{
    size >>= 1;

    int shift = 0;
    while( size > 0 )
    {
        size >>= 1;
        shift++;
    }
    return 31 - shift; // 2 GB per file max
}
}// namespace


/**
 *  Same as in dataHDDIOOctree
 */
void DataHDDIOTensorQuantized2G::_getFileNameAndOffset( const uint32_t     id,
                                                   std::string& name, uint32_t&  offset )
{
    const uint32_t bs         = getMaxBlockSize();
    const uint32_t pow2Blocks = _pow2NumberOfBlocks( bs );

    const uint32_t fileNum = id >> pow2Blocks;

    offset = ( id - ( fileNum << pow2Blocks )) * bs;

    name.append( _baseName );
    name.append( strUtil::toString( fileNum ));
    name.append( ".raw" );
}


// similar to dataHDDIOOctree
bool DataHDDIOTensorQuantized2G::read( const uint32_t id, void* dst )
{
    assert( isBlockSizeValid() );
    assert( id != 0 );

          std::string fName = getDataFileDir();
          uint32_t    offset;
    const uint32_t    blockSize = getMaxBlockSize();
    _getFileNameAndOffset( id-1, fName, offset );

    util::InFile inFile;

    if( inFile.open( fName, std::ios::binary, true ) &&
        inFile.read( offset, blockSize, dst ))
        return true;

    return false;
}


// similar to dataHDDIOOctree
bool DataHDDIOTensorQuantized2G::write( const uint32_t id, const void* src )
{
    assert( isBlockSizeValid() );

          std::string fName = getDataFileDir();
          uint32_t    offset;
    const uint32_t    blockSize = getMaxBlockSize();
    _getFileNameAndOffset( id-1, fName, offset );

// Create / extend a file if necessary
    if( util::fileSize( fName ) < offset+blockSize )
    {
        if( !util::extendFile( fName, offset+blockSize ))
            return false;
        LOG_INFO << "file " << fName.c_str() << " extended file up to: " << offset+blockSize << std::endl;
    }

    util::OutFile outFile;
    if( outFile.open( fName, std::ios_base::in | std::ios_base::binary, true ) &&
        outFile.write( offset, blockSize, src ))
        return true;

    return false;
}


bool DataHDDIOTensorQuantized2G::allocateAllFiles( const uint32_t maxId )
{
    if( maxId < 1 )
    {
        LOG_INFO << "Nothing to allocate " << std::endl;
        return true;
    }

    const uint32_t bs      = getMaxBlockSize();
    const uint64_t nBlocks = 1 << _pow2NumberOfBlocks( bs );
    if( nBlocks < 1 )
    {
        LOG_ERROR << "Number of blocks per file can't be 0!" << std::endl;
        return false;
    }

    std::vector<byte> dVec( bs );
    if( dVec.size() != bs )

    {
        LOG_ERROR << "Can't allocate " << bs<< " bytes" << std::endl;
        return false;
    }
    byte* data = &dVec[0];

    // create/extend the last file
    if( !write( maxId, data ) )
        return false;

    // create/extend all other files
    uint64_t currentId = nBlocks;
    while( currentId < maxId )
    {
        if( !write( currentId, data ))
            return false;

        currentId += nBlocks;
    }

    return true;
}


//============================ DataHDDIOTensorQuantizedErrors2G functions ============================


VolumeTreeBaseSPtr DataHDDIOTensorQuantizedErrors2G::_createTree()
{
    return VolumeTreeBaseSPtr(
        new VolumeTreeTensorErrors( getSourceDims(), getBlockDim(), _nodesIds, _ranks, _scales, _errors ));
}


DataHDDIOTensorQuantizedErrors2G::DataHDDIOTensorQuantizedErrors2G( const VolumeFileInfo& fileInfo, bool initTree )
    : DataHDDIOTensorQuantized2G( fileInfo, false )
{
    assert( _ranks );
    _errors = RankErrorsVecSPtr( new RankErrorsVec( _ranks->size() ));

    if( initTree )
        _initTree();
}


bool DataHDDIOTensorQuantizedErrors2G::_readIdsRanksScales( )
{
    assert( _nodesIds && _ranks && _scales );

    const size_t size = _nodesIds->size();

    if( _ranks->size()  != size ||
        _scales->size() != size )
    {
        LOG_ERROR << "Size of _ranks or _scales is incorect " << std::endl;
        return false;
    }

    memset( &((*_nodesIds)[ 0]), 0, size*sizeof((*_nodesIds)[0]));
    memset( &((*_ranks)[    0]), 0, size*sizeof((*_ranks)[   0]));
    memset( &((*_scales)[   0]), 0, size*sizeof((*_scales)[  0]));

    std::string fName = getRanksFileName();

// Read ranks, build ids
    std::vector<double> data( size );
    const uint32_t bytesToRead = data.size()*sizeof(data[0]);

    util::InFile inFile;
    if( !inFile.open( fName, std::ios::binary, true ) ||
        !inFile.read( bytesToRead, bytesToRead, &data[0] ))
        return false;

    byte     maxRank   = 0;
    uint32_t currentId = 0;
    for( size_t i = 0; i < size; ++i )
    {
        int32_t rank = static_cast<int32_t>( data[i]+0.5 );
        if( rank < 0 || rank > 255 )
        {
            LOG_ERROR << "Incorrect rank? (" << rank << ")" << std::endl;
            break;
        }
        if( rank > 0 )
        {
            currentId++;
            (*_nodesIds)[i] = currentId;
            (*_ranks)[   i] = rank;
            if( maxRank < rank )
                maxRank = rank;
        }
    }
    setMaxRankDim( maxRank );

    // check that all non zero ranks have the same size
    for( size_t i = 0; i < size; ++i )
        if( (*_ranks)[i] != 0 && (*_ranks)[i] != maxRank )
        {
            LOG_ERROR << "One of the non-zero ranks is not equal to maxRank!" << std::endl;
            abort();
        }

    // Read scales
    if( !inFile.read( bytesToRead*4, bytesToRead, &data[0] ))
        return false;

    for( size_t i = 0; i < size; ++i )
    {
        if( (*_ranks)[i] > 0 )
            (*_scales)[i] = log2f( 1.0 + data[i] ) / 127.f;
    }

    // Read errors
    data.resize( size*6 );
    if( !inFile.read( bytesToRead*8, bytesToRead*6, &data[0] ))
        return false;

    // Print min & max
    double minError = data[0];
    double maxError = data[0];
    for( size_t i = 1; i < data.size(); ++i )
        if( data[i] > 0.000001f )
        {
            if( minError > data[i] ) minError = data[i]; else
            if( maxError < data[i] ) maxError = data[i];
        }
    std::cout << "min error: " << minError << " max error: " << maxError
              << " max-min: " << maxError-minError << std::endl;
    // fill errors
    const size_t SZ = 6;
    double* d[SZ];
    d[0] = &data[0];
    for( size_t i = 1; i < SZ; ++i )
        d[i] = d[i-1] + size;

    double multiplier = 1000.f / maxError;
    int errNum = 0;
    for( size_t i = 0; i < size; ++i )
        if( (*_ranks)[i] != 0 )
        {
            (*_errors)[i].set( d[0][i], d[1][i], d[2][i], d[3][i], d[4][i], d[5][i], multiplier );
            if( d[0][i] + d[1][i] + d[2][i] + d[3][i] + d[4][i] + d[5][i] < 0.0001 && ++errNum <= 20 )
                LOG_ERROR << "errors are zeros for block " << i << std::endl;
        }
    if( errNum > 20 )
        LOG_ERROR << "and " << errNum-20 << " more similar errors " << std::endl;

#if 0 // print first elements of rank array
    for( size_t i = 0; i < 1+8+64; ++i )
    {
        std::cout << "i: " << i << " id: " << (int)nodes[i].id << " rank: " << (int)ranks[i] << " scale: " << scales[i] << std::endl;
    }
#endif
    return true;
}


} //namespace massVolVis






