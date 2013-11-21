
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__OCTREE_INFO_H
#define MASS_VOL__OCTREE_INFO_H

#include <msv/types/box.h>
#include <msv/types/vec2.h>
#include <msv/types/vec3.h>
#include <msv/types/types.h>

#include <msv/tree/rankErrors.h>

#include <msv/types/bits.h>

#include <vector>

#include <boost/shared_ptr.hpp>


namespace massVolVis
{

class VolumeTreeBase;
class DataHDDIO;

typedef boost::shared_ptr< DataHDDIO > DataHDDIOSPtr;

/**
 *  Information about stored data
 */
class VolumeFileInfo
{
public:
    enum DataType
    {
        VERSION      = Bits::B_1,
        SOURCE_DIMS  = Bits::B_2,
        BLOCK_DIM    = Bits::B_3,
        BORDER_DIM   = Bits::B_4,
        BYTES        = Bits::B_5,
        COMPRESSION  = Bits::B_6,
        DATA_FILE    = Bits::B_7,
        TF_FILE      = Bits::B_8,
        MAX_RANK     = Bits::B_9,  // Tensor stuff
        U123_DIMS    = Bits::B_10, // Tensor stuff
        U_OFFSETS    = Bits::B_11  // Tensor stuff
    };

    enum CompressionType
    {
        NONE                       = 0,
        TENSOR                     = 1,
        TENSOR_QUANTIZED           = 2,
        TENSOR_QUANTIZED_2G        = 3,
        TENSOR_QUANTIZED_ERRORS_2G = 4
    };

    VolumeFileInfo();
    explicit VolumeFileInfo( const VolumeFileInfo& fileInfo );
    virtual ~VolumeFileInfo();

    virtual void reset();
    virtual void setDefaults();

    virtual void setVersion(      const int             version  ) { _version      = version;  _setAttribute( VERSION     ); }
    virtual void setSourceDims(   const Vec3_ui16&      dims     ) { _srcDims      = dims;     _setAttribute( SOURCE_DIMS ); }
    virtual void setU123SrcDims(  const Vec2_ui16&      dims     ) { _u123SrcDims  = dims;     _setAttribute( U123_DIMS   ); }
    virtual void setUOffsets(     const std::string&    uOffsets ) { _uOffsets     = uOffsets; _setAttribute( U_OFFSETS   ); }
    virtual void setBlockDim(     const uint32_t        blockDim ) { _blockDim     = blockDim; _setAttribute( BLOCK_DIM   ); }
    virtual void setMaxRankDim(   const uint32_t        rankDim  ) { _maxRankDim   = rankDim;  _setAttribute( MAX_RANK    ); }
    virtual void setBorderDim(    const byte            border   ) { _border       = border;   _setAttribute( BORDER_DIM  ); }
    virtual void setBytesNum(     const byte            bytes    ) { _bytes        = bytes;    _setAttribute( BYTES       ); }
    virtual void setCompression(  const CompressionType cmpr     ) { _compression  = cmpr;     _setAttribute( COMPRESSION ); }
    virtual void setDataFileName( const std::string&    fileName );
    virtual void setTFFileName(   const std::string&    fileName ) { _tfFileName   = fileName; _setAttribute( TF_FILE     ); }

    virtual uint32_t getBlockSize_() const;

    /**
     * If compression is TENSOR, then this function might update maxRank.
     *
     * @param [out] hddIO Data IO, based on parameters and compression
     */
    virtual DataHDDIOSPtr createDataHDDIO( bool initTree = true );

    int                 getVersion()      const { return _version;      }
    const Vec3_ui16&    getSourceDims()   const { return _srcDims;      }
    const Vec2_ui16&    getU123SrcDims()  const { return _u123SrcDims;  }
    const Vec2_ui16     getU123Dims()     const { return Vec2_ui16( _maxRankDim, _u123SrcDims.h ); }
    const std::string&  getUOffsets()     const { return _uOffsets;     }
    uint32_t            getBlockDim()     const { return _blockDim;     }
    uint32_t            getMaxRankDim()   const { return _maxRankDim;   }
    byte                getBorderDim()    const { return _border;       }
    byte                getBytesNum()     const { return _bytes;        }
    CompressionType     getCompression()  const { return _compression;  }
    const std::string&  getDataFileName() const { return _dataFileName; }
    const std::string&  getDataFileDir()  const { return _dataFileDir;  }
    const std::string&  getTFFileName()   const { return _tfFileName;   }
          std::string   getHistorgamFileName() const { return getDataFileDir() + "/hist.raw"; }

    uint32_t            getBlockAndBordersDim() const { return _blockDim + _border*2; }

    bool load( const std::string& file );        //!< Load data info from a file
    bool save( const std::string& file )  const; //!< Save data info to a file

    bool isAttributeSet( const DataType dataType ) const;

    friend std::ostream& operator<<( std::ostream& out, const VolumeFileInfo& info );

private:
    void _updateBlockSize();
    void _setAttribute( const DataType dataType );

    uint32_t        _attributes; //!< show if some attributes were loaded

    int             _version;       //!< config file version
    Vec3_ui16       _srcDims;       //!< original dimensions of the data
    Vec2_ui16       _u123SrcDims;   //!< Initial tensor U1/U2/U3 dimensions (same for all matrixes)
    std::string     _uOffsets;      //!< list of column offsets for different levels of tree in tensor compression
    uint32_t        _blockDim;      //!< block dimention (we have cubes, also this is constant for all blocks)
    uint32_t        _maxRankDim;    //!< maximum rank dimention for tensors (we have cubes, also this is constant for all blocks)
    byte            _border;        //!< border size (borders are symmetrical for all dimensions)
    byte            _bytes;         //!< number of bytes per value (usually 1 or 2, i.e. 8 or 16 bit data)
    CompressionType _compression;   //!< type of compression used for the data
    std::string     _dataFileName;  //!< for raw data - name of the file, for octree - name of the folder
    std::string     _dataFileDir;   //!< name of the folder
    std::string     _tfFileName;    //!< Transfer Function file name

};

}// namespace massVolVis

#endif //MASS_VOL__OCTREE_INFO_H