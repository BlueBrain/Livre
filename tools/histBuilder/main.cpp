
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */


#include "../../volVis/asyncFetcher/compression/tensorCPU.h"

#include <msv/IO/dataHDDIOTensor.h>
#include <msv/tree/volumeTreeTensor.h>
#include <msv/util/debug.h>
#include <msv/util/hlp.h>
#include <msv/util/str.h>  // toString

#include <tclap/CmdLine.h>

#include <math.h>

#include <exception>
#include <assert.h>

#include <cstring> // memset

#ifdef __APPLE__
    #include <sys/types.h>
#endif

using namespace massVolVis;


typedef boost::shared_ptr<       VolumeTreeTensor >      VolumeTreeTensorSPtr;
typedef boost::shared_ptr< const VolumeTreeTensor > constVolumeTreeTensorSPtr;

namespace
{
const std::string _help( "TENSOR_QUANTIZED to TENSOR_QUANTIZED_2G converter\n" );


void _decompressBlock( const uint32_t treePos, const DataHDDIOTensorBase* hddIO, constVolumeTreeTensorSPtr tree,
                      const std::vector<float>& u123, std::vector<byte>& dst, std::vector<byte>& src  )
{
/*    const bool tensorQuantized = dynamic_cast<const DataHDDIOTensorQuantized*>( hddIO ) == 0 ? false : true;

    const byte     level   = VolumeTreeBase::getLevelFromPos( treePos );
    const uint32_t blockDim = hddIO->getBlockDim()+hddIO->getBorderDim()*2;
            Vec3_i32 bPos    = tree->getRelativeCoordinates( treePos ).s;
    if( tensorQuantized ) // add inner borders
        bPos += bPos / hddIO->getBlockDim() * hddIO->getBorderDim()*2;
    const uint32_t uStride = hddIO->getU123Dims().w*3;

    const uint32_t uOffsetBase = hddIO->getU1Offset( level );
    const uint32_t u1Offset    = uStride*bPos.x;
    const uint32_t u2Offset    = uStride*bPos.y + hddIO->getU123Dims().w;
    const uint32_t u3Offset    = uStride*bPos.z + hddIO->getU123Dims().w*2;

    const byte   coreDim  = hddIO->getNodeRank( treePos );
    const float  scale    = tree->getNodeScale( treePos );

    int rank = coreDim;

    static std::vector<float> tmp1( blockDim*blockDim*blockDim );
    static std::vector<float> tmp2( blockDim*blockDim*blockDim );

    if( tensorQuantized )
    {
        cpu::decompressTensorQuantized2(
                            &dst[0],
                            &tmp1[0],
                            &tmp2[0],
                            &src[0],
                            &u123[ uOffsetBase + u1Offset ],
                            &u123[ uOffsetBase + u2Offset ],
                            &u123[ uOffsetBase + u3Offset ],
                            coreDim, rank, blockDim, uStride, scale,
                            hddIO->getBytesNum());
    }else
    {
        cpu::decompressTensorBasic(
                            &dst[0],
                            &tmp1[0],
                            &tmp2[0],
                            reinterpret_cast<const float*>(&src[0]),
                            &u123[ uOffsetBase + u1Offset ],
                            &u123[ uOffsetBase + u2Offset ],
                            &u123[ uOffsetBase + u3Offset ],
                            coreDim, blockDim, uStride, scale,
                            hddIO->getBytesNum());
    }

    */
}


uint32_t getHistogram( const uint8_t* src, uint32_t size )
{
    uint32_t result = 0;
    for( uint i = 0; i < size; ++i )
    {
        if( src[i] == 0 )
            continue;
        result |= 1 << (31 - ( src[i] >>  3 )); // 5 most significant bits
    }
    return result;
}


uint32_t getHistogram( const uint16_t* src, uint32_t size )
{
    uint32_t result = 0;
    for( uint i = 0; i < size; ++i )
    {
        if( src[i] == 0 )
            continue;
        result |= 1 << (31 - ( src[i] >> 11 )); // 5 most significant bits
    }
    return result;
}


uint32_t getHistogram( const uint8_t* src, uint32_t voxelsPerBlock, bool is8bits )
{
    if( is8bits )
        return getHistogram( src, voxelsPerBlock );
    else
        return getHistogram( reinterpret_cast<const uint16_t*>( src ), voxelsPerBlock );
}

} // namespace


int main( int argc, char **argv )
{
/*    try
    {
        TCLAP::CmdLine command( _help );

        TCLAP::ValueArg<std::string> dstArg( "d", "dst", "destination file",
                                       false, "", "string", command );

        TCLAP::ValueArg<std::string> srcArg( "s", "src", "source file",
                                       false, "", "string", command );

        command.parse( argc, argv );

        VolumeFileInfo srcInfo;
        if( !srcArg.isSet( ) || !srcInfo.load( srcArg.getValue( )))
            throw( TCLAP::ArgException( "Source file is missing or can't be loaded", "src" ));

        DataHDDIOSPtr src = srcInfo.createDataHDDIO();
        DataHDDIOTensorBase* srcTensor = dynamic_cast<DataHDDIOTensorBase*>( src.get() );

        const bool isCompressed = src->getCompression() != VolumeFileInfo::NONE;
        const bool is8bits      = src->getBytesNum() == 1;

        assert( !isCompressed || srcTensor );

        std::string dstFile;
        if( dstArg.isSet( ))
            dstFile = dstArg.getValue();
        else
            dstFile = src->getDataFileDir() + "/hist.raw";


        LOG_INFO << "Going to analyse file:" << std::endl << *src << std::endl;

        LOG_INFO << "Aggregating histograms from leafs..." << std::endl;

        constVolumeTreeTensorSPtr tree = boost::dynamic_pointer_cast<const VolumeTreeTensor>( src->getTree() );
        assert( !isCompressed || tree );

        std::vector<uint32_t> hists( tree->getSize(), 0 );

        const uint32_t voxelsPerBlock   = hlpFuncs::cubed( src->getBlockAndBordersDim() );
        const uint32_t sizeCompressed   = src->getMaxBlockSize();
        const uint32_t sizeDecompressed = src->getBytesNum() * voxelsPerBlock;
        std::vector<byte> srcData( sizeCompressed,   0 );
        std::vector<byte> dstData( sizeDecompressed, 0 );
        assert( srcData.size() == sizeCompressed   );
        assert( dstData.size() == sizeDecompressed );

        std::vector<float> u123; //!< U1/U2/U3 matrixes for Tensor deconstruction
        if( isCompressed )
        {
            srcTensor->readU123( u123 );
        }

        uint32_t leafFirstChild = 0;
        while( tree->hasChildren( leafFirstChild ))
        {
            leafFirstChild = VolumeTreeBase::getChild( leafFirstChild );
        }

        uint32_t validBlocks   = 0;
        for( uint32_t pos = leafFirstChild; pos < tree->getSize(); ++pos )
            if( tree->getNodeId( pos ))
                ++validBlocks;

        uint32_t counter       = 0;
        int64_t  oldPercents   = -1;
        uint32_t blocksPercentage = hlpFuncs::myClip( validBlocks/100u, 100u, 1000u );

        for( uint32_t pos = leafFirstChild; pos < tree->getSize(); ++pos )
        {
            const uint32_t nodeId = tree->getNodeId( pos );
            if( !nodeId )
                continue;
            //else decompress the node and compute histogram
            if( !isCompressed )
            {
                src->read( nodeId, &dstData[0] );
            }else
            {
                src->read( nodeId, &srcData[0] );
                _decompressBlock( pos, srcTensor, tree, u123, dstData, srcData );
            }

            hists[ pos ] = getHistogram( &dstData[0], voxelsPerBlock, is8bits );

            // count how many blocks were already processed
            if( (++counter) % blocksPercentage == 0 )
            {
                int64_t percents = int64_t(counter*100/validBlocks);

                if( percents != oldPercents )
                {
                    LOG_INFO << std::endl;
                    LOG_INFO << "analysing blocks (" << counter << "/" << validBlocks << "): "
                                << percents << "%" << std::flush;
                    oldPercents = percents;
                }else
                    LOG_INFO << "." << std::flush;
            }
        }
        LOG_INFO << std::endl;

        LOG_INFO << "Updating histograms for parents..." << std::endl;
        // basically parentHist = max( children's hists )
        for( uint32_t posTmp = leafFirstChild; posTmp > 0; --posTmp )
        {
            const uint32_t pos = posTmp-1;

            if( !tree->getNodeId( pos ))
                continue;
            assert( tree->hasChildren( pos ));

            uint32_t childrenPos = VolumeTreeBase::getChild( pos );
            for( uint32_t i = childrenPos; i < childrenPos+8; ++i )
                hists[pos] |= hists[i];
        }

        LOG_INFO_ << "Saving hist file." << std::endl;

        // TODO: replace with a single writeFile function
        std::ofstream os;
        os.open( dstFile.c_str(), std::ios_base::out | std::ios_base::binary  );
        if( !os.is_open( ))
        {
            LOG_ERROR << "Can't open file to write: " << dstFile.c_str() << std::endl;
            throw "";
        }
        os.seekp( 0, std::ios::beg );
        if( os.tellp() != 0 )
        {
            LOG_ERROR << "Can't proceed to the offset: " << 0 << " to write file: " << dstFile.c_str() << std::endl;
            throw "";
        }
        os.write( reinterpret_cast<const char*>(&hists[0]), hists.size()*sizeof(hists[0]) );
        if( os.fail( ))
        {
            LOG_ERROR << "Some error happen during writing to a file: " << dstFile.c_str() 
                    << " with the offset: " << 0
                    << " of " << hists.size()*sizeof(hists[0]) << " bytes." << std::endl;
            throw "";
        }
        os.close();

        LOG_INFO_ << "Done converting." << std::endl;

        srcTensor  = 0;
    }
    catch( TCLAP::ArgException& exception )
    {
        LOG_ERROR   << "Command line parse error: " << exception.error()
                    << " for argument " << exception.argId() << std::endl;

    }
    catch( const char* e )
    {
        LOG_ERROR << "Error occure: " << e << std::endl;
    }
    catch( const std::string& e )
    {
        LOG_ERROR << "Error occure: " << e.c_str() << std::endl;
    }
*/
    return 0;
}


