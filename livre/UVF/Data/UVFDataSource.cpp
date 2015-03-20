/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *
 * This file is part of Livre <https://github.com/BlueBrain/Livre>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <livre/UVF/Data/UVFDataSource.h>

#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Data/LODNode.h>
#include <livre/core/Data/MemoryUnit.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wshadow"

#include <Tuvok/IO/UVF/UVF.h>
#include <Tuvok/IO/UVF/ExtendedOctree/ZlibCompression.h>
#include <Tuvok/IO/UVF/TOCBlock.h>
#include <Tuvok/Basics/LargeFileMMap.h>
#include <Tuvok/IO/uvfDataset.h>

#pragma GCC diagnostic pop

#define MINIMUM_NUMBEROF_NODES 8u
#define MAX_ACCEPTABLE_BLOCK_SIZE 512

namespace livre
{

namespace
{
   lunchbox::PluginRegisterer< UVFDataSource > registerer;
}

namespace detail
{

class UVFDataSource
{
public:
    UVFDataSource(  VolumeInformation& volumeInfo,
                    const VolumeDataSourcePluginData& initData )
        : _uvfTOCBlock( 0 ),
          _volumeInfo( volumeInfo )
    {
        try
        {
            const std::string& path = initData.getURI().getPath();
            _uvfDataSetPtr.reset(
                new tuvok::UVFDataset( path, MAX_ACCEPTABLE_BLOCK_SIZE, false,
                                       false ));
            // For to use with MMap
            _tuvokLargeMMapFilePtr.reset( new LargeFileMMap( path ));

            uint32_t depth = 0;
            UINTVECTOR3 lodSize;
            do
            {
                 lodSize = _uvfDataSetPtr->GetBrickLayout( depth++, 0 );
            }
            while( lodSize[0] >= MINIMUM_NUMBEROF_NODES &&
                   lodSize[1] >= MINIMUM_NUMBEROF_NODES &&
                   lodSize[2] >= MINIMUM_NUMBEROF_NODES  );

            const UINTVECTOR3 tuvokBricksInRootLod =
                    _uvfDataSetPtr->GetBrickLayout( depth, 0 );

            _volumeInfo.rootNode =
                    RootNode( depth, Vector3ui( tuvokBricksInRootLod[0],
                                                tuvokBricksInRootLod[1],
                                                tuvokBricksInRootLod[2] ));

            _volumeInfo.isBigEndian =
                    _uvfDataSetPtr->GetUVFFile()->GetGlobalHeader().bIsBigEndian;
            _volumeInfo.compCount = _uvfDataSetPtr->GetComponentCount();

            const uint32_t bitWidth = _uvfDataSetPtr->GetBitWidth();
            if( _uvfDataSetPtr->GetIsFloat() )
            {
                if( bitWidth == 32 )
                    _volumeInfo.dataType = DT_FLOAT32;
                else if( bitWidth == 64 )
                    _volumeInfo.dataType = DT_FLOAT64;
            }
            else if( !_uvfDataSetPtr->GetIsSigned() )
            {
                if( bitWidth == 32 )
                    _volumeInfo.dataType = DT_UINT32;
                else if( bitWidth == 16 )
                    _volumeInfo.dataType = DT_UINT16;
                else if(bitWidth == 8 )
                    _volumeInfo.dataType = DT_UINT8;
            }
            else
            {
                if( bitWidth == 32 )
                    _volumeInfo.dataType = DT_INT32;
                else if( bitWidth == 16 )
                    _volumeInfo.dataType = DT_INT16;
                else  if( bitWidth == 8 )
                    _volumeInfo.dataType = DT_INT8;
            }

            const UINTVECTOR3& maxBrickSize =_uvfDataSetPtr->GetMaxBrickSize();
            _volumeInfo.maximumBlockSize = Vector3ui( maxBrickSize[0],
                                                      maxBrickSize[1],
                                                      maxBrickSize[2] );

            const UINTVECTOR3& overlap = _uvfDataSetPtr->GetBrickOverlapSize( );
            _volumeInfo.overlap = Vector3i( overlap[0], overlap[1], overlap[2] );

            const UINT64VECTOR3& domainSize = _uvfDataSetPtr->GetDomainSize( );
            _volumeInfo.worldSpacePerVoxel = 1.0f / (float)domainSize.maxVal();

            _volumeInfo.voxels = Vector3i( domainSize[0],
                                           domainSize[1],
                                           domainSize[2] );
            _volumeInfo.worldSize = Vector3f( domainSize[0],
                                              domainSize[1],
                                              domainSize[2] ) / (float)domainSize.maxVal();

            _readTOCBlock( initData.getURI().getPath());
         }
        catch( ... )
        {
            LBTHROW( std::runtime_error( "UVF data format initialization failed" ));
        }
    }

    ~UVFDataSource()
    {
        if( _tuvokLargeMMapFilePtr )
            _tuvokLargeMMapFilePtr->close();
    }

    void _readTOCBlock( const std::string& uri )
    {
        const UVF* uvfFile = _uvfDataSetPtr->GetUVFFile();
        for( uint32_t iBlocks = 0; iBlocks < uvfFile->GetDataBlockCount(); ++iBlocks )
        {
            if( uvfFile->GetDataBlock( iBlocks )->GetBlockSemantic() ==
                    UVFTables::BS_TOC_BLOCK )
            {
                _uvfTOCBlock = static_cast< TOCBlock* >(
                            uvfFile->GetDataBlock(iBlocks).get() );
                break;
            }
        }

        /*
         * There is no API to get the offset and before reading data,
         * file should be seeked correctly.
         */
        const GlobalHeader& globalHeader = uvfFile->GetGlobalHeader();
        _offset = sizeof( bool ) + 4 * sizeof( std::uint64_t ) +
                globalHeader.vcChecksum.size() + 8;

        LargeRAWFile_ptr filePtr( new LargeRAWFile( uri ) );
        filePtr->Open( );
        filePtr->SeekPos( _offset );

        std::uint64_t temp;

        filePtr->ReadData( temp, _volumeInfo.isBigEndian );

        std::string strBlockId;
        filePtr->ReadData( strBlockId, temp );

        filePtr->ReadData( temp, _volumeInfo.isBigEndian );
        filePtr->ReadData( temp, _volumeInfo.isBigEndian );
        filePtr->ReadData( temp, _volumeInfo.isBigEndian );

        _offset = filePtr->GetPos();

        filePtr->Close();
    }


    MemoryUnitPtr getData( const LODNode& node )
    {
        const Vector3i& minPos = node.getAbsolutePosition();
        const UINTVECTOR3& tuvokBricksInThisLod =
                _uvfDataSetPtr->GetBrickLayout(
                    _treeLevelToTuvokLevel( node.getRefLevel( )), 0 );
        const Vector3i bricksInThisLod(  tuvokBricksInThisLod.x,
                                         tuvokBricksInThisLod.y,
                                         tuvokBricksInThisLod.z );

        uint32_t brickIndex = _getBrickIndex( minPos[ 0 ],
                                              minPos[ 1 ],
                                              minPos[ 2 ],
                                              bricksInThisLod );

        MemoryUnitPtr memUnitPtr;
        switch( _volumeInfo.dataType )
        {
            case  DT_FLOAT32 :
              memUnitPtr = _tuvokBrickToMemoryUnit< float >( node, brickIndex );
              break;
            case  DT_FLOAT64 :
              memUnitPtr = _tuvokBrickToMemoryUnit< double >( node, brickIndex );
              break;
            case  DT_UINT8 :
              memUnitPtr = _tuvokBrickToMemoryUnit< uint8_t >( node,brickIndex );
              break;
            case  DT_UINT16 :
              memUnitPtr = _tuvokBrickToMemoryUnit< uint16_t >( node, brickIndex );
              break;
            case  DT_UINT32 :
              memUnitPtr = _tuvokBrickToMemoryUnit< uint32_t >( node, brickIndex );
              break;
            case  DT_INT8 :
              memUnitPtr = _tuvokBrickToMemoryUnit< int8_t >( node, brickIndex );
              break;
            case  DT_INT16 :
              memUnitPtr = _tuvokBrickToMemoryUnit< int16_t >( node, brickIndex );
              break;
            case  DT_INT32 :
              memUnitPtr = _tuvokBrickToMemoryUnit< int32_t >( node, brickIndex );
              break;
            case  DT_UNDEFINED :
              LBERROR << "Undefined data type" << std::endl;
              break;
        }
        return memUnitPtr;
    }

    template< class T >
    MemoryUnitPtr _tuvokBrickToMemoryUnit( const LODNode& node,
                                           const uint32_t brickIndex ) const
    {
        const tuvok::BrickKey brickKey =
                tuvok::BrickKey( 0, _treeLevelToTuvokLevel(
                                            node.getRefLevel( )), brickIndex );

        const UINT64VECTOR4 coords = _uvfDataSetPtr->KeyToTOCVector( brickKey );
        const TOCEntry& blockInfo = _uvfTOCBlock->GetBrickInfo( coords );

        MemoryUnitPtr memUnitPtr;

        if( blockInfo.m_eCompression == CT_NONE )
        {
             const std::uint64_t offset = _offset + blockInfo.m_iOffset;
             const std::uint64_t length = blockInfo.m_iLength;
             const unsigned char* dataPtr =
                     (const unsigned char*)_tuvokLargeMMapFilePtr->rd(
                                                        offset, length ).get( );

             memUnitPtr.reset( new ConstMemoryUnit( dataPtr,
                                        blockInfo.m_iLength / sizeof( T ) ) );
        }
        else if( blockInfo.m_eCompression == CT_ZLIB )
        {
            const Vector3i dimensions = node.getVoxelBox().getDimension();
            const uint32_t uncompressedSize = dimensions[ 0 ] *
                                              dimensions[ 1 ] *
                                              dimensions[ 2 ] * _volumeInfo.compCount;

            std::vector< T > tuvokData;
            tuvokData.resize( uncompressedSize );
            const void* dataPtr =
                    _tuvokLargeMMapFilePtr->rd( _offset + blockInfo.m_iOffset,
                                                blockInfo.m_iLength ).get( );

            // The below piece of "art" is because of the zDecompress(...)
            // API from TUVOK, a ref to a shared ptr ? wow
            // void zDecompress(std::shared_ptr<uint8_t> src,
            // std::shared_ptr<uint8_t>& dst,size_t uncompressedBytes)
            std::shared_ptr< std::uint8_t > src( (std::uint8_t *)dataPtr,
                                                    DontDeleteObject< std::uint8_t >() );
            std::shared_ptr< std::uint8_t > dst( (std::uint8_t *)&tuvokData[ 0 ],
                                                   DontDeleteObject< std::uint8_t >() );
            zDecompress( src, dst, uncompressedSize );

            AllocMemoryUnit *allocUnit = new AllocMemoryUnit( );
            allocUnit->allocAndSetData< T >( tuvokData );
            memUnitPtr.reset( allocUnit );
        }
        else
        {
            const Vector3i dimensions = node.getVoxelBox().getDimension();
            const uint32_t uncompressedSize = dimensions[ 0 ] *
                                              dimensions[ 1 ] *
                                              dimensions[ 2 ] * _volumeInfo.compCount;

            std::vector< T > tuvokData;
            tuvokData.resize( uncompressedSize );

            AllocMemoryUnit *allocUnit = new AllocMemoryUnit( );
            allocUnit->allocAndSetData< T >( tuvokData );
            memUnitPtr.reset( allocUnit );
        }

        return memUnitPtr;
    }

    void internalNodeToLODNode( const NodeId& internalNode,
                                LODNode& lodNode ) const
    {
        const uint32_t lod = _treeLevelToTuvokLevel( internalNode.getLevel() );

        const UINTVECTOR3& tuvokBricksInLod =
                _uvfDataSetPtr->GetBrickLayout( lod, 0 );

        const Vector3ui& localBlockPosition = internalNode.getPosition();
        if( localBlockPosition[ 0 ] >= tuvokBricksInLod[ 0 ] ||
            localBlockPosition[ 1 ] >= tuvokBricksInLod[ 1 ] ||
            localBlockPosition[ 2 ] >= tuvokBricksInLod[ 2 ] )
        {
            // UVF format is not a perfect octree but its octree structure is a
            // subset of perfect octree
            return;
        }

        const Vector3ui bricksInLod( tuvokBricksInLod.x,
                                     tuvokBricksInLod.y,
                                     tuvokBricksInLod.z );

        const tuvok::BrickKey brickKey( 0,
                                        lod,
                                        _getBrickIndex( localBlockPosition.x(),
                                                        localBlockPosition.y(),
                                                        localBlockPosition.z(),
                                                        bricksInLod ));

        tuvok::BrickMD brickInfo = _uvfDataSetPtr->GetBrickMetadata( brickKey );

        FLOATVECTOR3 vScale( float( _uvfDataSetPtr->GetScale().x ),
                             float( _uvfDataSetPtr->GetScale().y ),
                             float( _uvfDataSetPtr->GetScale().z ) );

        const UINT64VECTOR3& vDomainSize = _uvfDataSetPtr->GetDomainSize( lod );

        const FLOATVECTOR3 vDomainSizeCorrectedScale =
                vScale * FLOATVECTOR3( vDomainSize ) / float(vDomainSize.maxVal());
        vScale /= vDomainSizeCorrectedScale.maxVal();

        brickInfo.extents = brickInfo.extents * vScale;
        brickInfo.center = brickInfo.center * vScale;

        const FLOATVECTOR3 position = brickInfo.center - brickInfo.extents / 2.0;

        const Vector3f boxMin( position[ 0 ], position[ 1 ], position[ 2 ] );
        const Vector3f boxMax = boxMin + Vector3f( brickInfo.extents[ 0 ],
                                                   brickInfo.extents[ 1 ],
                                                   brickInfo.extents[ 2 ] );
        const Boxf worldBox( boxMin, boxMax );
        const Vector3i blockSize = Vector3i( brickInfo.n_voxels[ 0 ],
                                             brickInfo.n_voxels[ 1 ],
                                             brickInfo.n_voxels[ 2 ] );

        lodNode = LODNode( internalNode,
                           _volumeInfo.rootNode.getDepth(),
                           blockSize,
                           worldBox );
    }

    uint32_t _getBrickIndex( const uint32_t x,
                             const uint32_t y,
                             const uint32_t z,
                             const Vector3i &max ) const
    {
        return x + y * max[ 0 ] + z * max[ 0 ] * max[ 1 ];
    }

    uint32_t _tuvokLODLevelToTreeLevel( const uint32_t tuvokLevel,
                                        const uint32_t depth ) const
    {
        return depth - tuvokLevel - 1;
    }


    uint32_t _treeLevelToTuvokLevel( const uint32_t treeLevel ) const
    {
        return _volumeInfo.rootNode.getDepth() - treeLevel - 1;
    }

    TOCBlock* _uvfTOCBlock;
    uint64_t _offset;

    typedef boost::scoped_ptr< tuvok::UVFDataset > UVFDatasetPtr;
    UVFDatasetPtr _uvfDataSetPtr;

    typedef boost::scoped_ptr< LargeFileMMap > LargeFileMMapPtr;
    LargeFileMMapPtr _tuvokLargeMMapFilePtr;

    VolumeInformation& _volumeInfo;
};

}

UVFDataSource::UVFDataSource( const VolumeDataSourcePluginData& initData )
    : _impl( new detail::UVFDataSource( _volumeInfo, initData ))
{
}

UVFDataSource::~UVFDataSource()
{
    delete _impl;
}


bool UVFDataSource::handles( const VolumeDataSourcePluginData& initData )
{
    return initData.getURI().getScheme() == "uvf";
}

MemoryUnitPtr UVFDataSource::getData( const LODNode& node )
{
    return _impl->getData( node );

}

void UVFDataSource::internalNodeToLODNode(
    const NodeId internalNode, LODNode& lodNode ) const
{
    return _impl->internalNodeToLODNode( internalNode, lodNode );
}


}
