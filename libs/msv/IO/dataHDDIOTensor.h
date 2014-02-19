
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__DATA_HDD_IO_TENSOR_H
#define MASS_VOL__DATA_HDD_IO_TENSOR_H

#include <msv/tree/nodeId.h>
#include <msv/tree/rankErrors.h>
#include "dataHDDIO.h"

namespace massVolVis
{

typedef std::vector<byte>  byteVec;
typedef std::vector<float> floatVec;

typedef boost::shared_ptr<   byteVec >   byteVecSPtr;
typedef boost::shared_ptr<  floatVec >  floatVecSPtr;
typedef boost::shared_ptr< NodeIdVec > NodeIdVecSPtr;

/**
 */
class DataHDDIOTensorBase : public DataHDDIO
{
public:
    virtual ~DataHDDIOTensorBase(){}

    virtual void setBaseFileName( std::string& baseName ) { _baseName = baseName; }

    virtual bool read( const uint32_t id, void* dst );

    virtual uint32_t getBlockSize_( const uint32_t pos ) const = 0; //!< Returns BS for particular block pos

    virtual byte getNodeRank( const uint32_t pos ) const;

    virtual void getU123Offsets( uint32_t& u1Offset, uint32_t& u2Offset, uint32_t& u3Offset, byte level, Vec3_i32 bPos ) const
    {
        const uint32_t uOffsetBase = getU1Offset( level );
        const uint32_t uStride = getU123Dims().w*3;

        _correctBlockPos( bPos );

        u1Offset = uOffsetBase + uStride*bPos.x;
        u2Offset = uOffsetBase + uStride*bPos.y + getU123Dims().w;
        u3Offset = uOffsetBase + uStride*bPos.z + getU123Dims().w*2;
    }

    virtual uint32_t getUStride() const { return getU123Dims().w*3; }

    /**
     * @param [out] depth depth of the tree 1..N
     * @return            dimensions of U1/U2/U3 matrixes after mip-mapped versions of them and borders are added
     */
    virtual Vec2_ui32 getFullU123Dims( byte& depth ) const = 0;

    Vec2_ui32 getFullU123Dims() const;

    Vec2_ui32 getFullU123SrcDims( byte& depth ) const;

    Vec2_ui32 getFullU123SrcDims() const { byte depth; return getFullU123SrcDims( depth ); }

    /**
     * Dimentions of the result is [getFullU123Dims().w*3, getFullU123Dims().h + borderDim*(treeLevels + 1)], i.e.
     * matrixes are returend within the same data.
     *
     * Note: this function will shrink U123Dims.w to the maxRank.
     *
     * @param [out] dst data of U1/U2/U3 matrixes together with mip-mapped versions (order is raw-wise)
     * @return true on success
     */
    virtual bool readU123( std::vector<float>& dst ) const = 0;

    virtual std::string getU123FileName( byte u ) const;
    virtual std::string getRanksFileName() const;

    /**
     * @param level      level of the tree
     * @return U1 matrix offset - border offset for a cpecific depth.
     */
    virtual uint32_t getU1Offset( byte level ) const = 0;


    virtual constVolumeTreeBaseSPtr getTree() const { return _tree; }

protected:
    explicit DataHDDIOTensorBase( const VolumeFileInfo& fileInfo );

    virtual void _correctBlockPos( Vec3_i32& ) const {}

    // call this from derived classes' constructors once
    bool _initTree();

    virtual VolumeTreeBaseSPtr _createTree();

    virtual bool _readIdsRanksScales() { throw "don't call"; }

    void _getFileNameAndOffset( const uint32_t id, std::string& name, uint32_t& offset );

    byteVecSPtr   _ranks;
    floatVecSPtr  _scales;
    NodeIdVecSPtr _nodesIds;

    std::string _baseName;  // base name of files withing the folder

    VolumeTreeBaseSPtr _tree;
};

/**
 * - LOD for U123 matrix is build by the class itself;
 * - Borders for blocks in U123 use overlaps of original U values;
 * - Scale factor is a float value;
 * - Order of blocks in the tree is linear (Susan's original slab by slab order).
 *   This order is corrected to Z-curve order as in the original octree;
 * - Each core is a separate file.
 */
class DataHDDIOTensorBasic : public DataHDDIOTensorBase
{
public:
    explicit DataHDDIOTensorBasic( const VolumeFileInfo& fileInfo, bool initTree );

    virtual Vec2_ui32 getFullU123Dims( byte& depth ) const;

    virtual uint32_t getU1Offset( byte level ) const;

    virtual uint32_t getBlockSize_( const uint32_t pos ) const;

    virtual bool readU123( std::vector<float>& dst ) const;

protected:
    virtual bool _readIdsRanksScales();
};

/**
 * - LOD for U123 is given in the U files;
 * - Berders for blocks are explicitly specified;
 * - Scale factor is stored as quantized to "byte" value;
 * - Order of blocks in the tree is linear (Susan's original slab by slab order).
 *   This order is corrected to Z-curve order as in the original octree;
 * - Each core is a separate file.
 */
class DataHDDIOTensorQuantized : public DataHDDIOTensorBase
{
public:
    explicit DataHDDIOTensorQuantized( const VolumeFileInfo& fileInfo, bool initTree );

    virtual Vec2_ui32 getFullU123Dims( byte& depth ) const;

    virtual uint32_t getU1Offset( byte level ) const;

    virtual uint32_t getBlockSize_( const uint32_t pos ) const;

    virtual bool readU123( std::vector<float>& dst ) const;

protected:
    virtual void _correctBlockPos( Vec3_i32& bPos ) const { bPos += bPos / getBlockDim() * getBorderDim()*2; }

    virtual bool _readIdsRanksScales();
};


/**
 * - Order of blocks follows Z-curve order as in the original octree;
 * - Cores are stored in 2GB files as in the original octree format.
 * - Currently all non-zero ranks have to be of the same size!
 */
class DataHDDIOTensorQuantized2G : public DataHDDIOTensorQuantized
{
public:
    explicit DataHDDIOTensorQuantized2G( const VolumeFileInfo& fileInfo, bool initTree );

    virtual void _getFileNameAndOffset( const uint32_t id, std::string& name, uint32_t& offset );

    virtual bool read( const uint32_t id, void* dst );
    virtual bool write( const uint32_t id, const void* src );

    virtual bool allocateAllFiles( const uint32_t maxId );

protected:
    virtual bool _readIdsRanksScales();
};


typedef boost::shared_ptr< RankErrorsVec > RankErrorsVecSPtr;

/**
 * - Order of blocks follows Z-curve order as in the original octree;
 * - Cores are stored in 2GB files as in the original octree format.
 * - Currently all non-zero ranks have to be of the same size!
 * - This reader creates VolumeTreeTensorErrors.
 */
class DataHDDIOTensorQuantizedErrors2G : public DataHDDIOTensorQuantized2G
{
public:
    explicit DataHDDIOTensorQuantizedErrors2G( const VolumeFileInfo& fileInfo, bool initTree );

protected:
    virtual VolumeTreeBaseSPtr _createTree();
    virtual bool _readIdsRanksScales();

    RankErrorsVecSPtr _errors;
};

}

#endif // MASS_VOL__DATA_HDD_IO_TENSOR_H


