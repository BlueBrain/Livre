
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */


#include <msv/IO/dataHDDIOTensor.h>
#include <msv/tree/volumeTreeTensor.h>
#include <msv/util/debug.h>
#include <msv/util/hlp.h>
#include <msv/util/fileIO.h>
#include <msv/util/str.h>  // toString

#include <tclap/CmdLine.h>

#include <math.h>

#include <exception>

#include <cstring> // memset
#include <boost/shared_ptr.hpp>

namespace
{
const std::string _help( "TENSOR_QUANTIZED to TENSOR_QUANTIZED_2G converter\n"
                         "   version 1.0\n" );
} // namespace


using namespace massVolVis;

typedef boost::shared_ptr<DataHDDIOTensorBase> DataHDDIOTensorBaseSPtr;
typedef boost::shared_ptr< const VolumeTreeBase > constVolumeTreeBaseSPtr;

int main( int argc, char **argv )
{
    try
    {
        TCLAP::CmdLine command( _help );

        TCLAP::ValueArg<std::string> dstArg( "d", "dst", "destination folder",
                                       false, "", "string", command );

        TCLAP::ValueArg<std::string> srcArg( "s", "src", "source file",
                                       false, "", "string", command );

        command.parse( argc, argv );

        VolumeFileInfo srcInfo;
        if( !srcArg.isSet( ) || !srcInfo.load( srcArg.getValue( )))
            throw( TCLAP::ArgException( "Source file is missing or can't be loaded", "src" ));

        if( srcInfo.getCompression() != VolumeFileInfo::TENSOR_QUANTIZED )
            throw( "Source has to be TENSOR_QUANTIZED" );

        if( !dstArg.isSet( ))
            throw( TCLAP::ArgException( "destination folder is missing", "dst" ));

        VolumeFileInfo dstInfo( srcInfo );
        dstInfo.setDataFileName( dstArg.getValue() );
        dstInfo.setCompression( VolumeFileInfo::TENSOR_QUANTIZED_2G );

        LOG_INFO << "Going to convert from:" << std::endl << srcInfo << std::endl;
        LOG_INFO << "to:"                    << std::endl << dstInfo << std::endl;

        DataHDDIOTensorBaseSPtr src = boost::dynamic_pointer_cast<DataHDDIOTensorBase>( srcInfo.createDataHDDIO() );
        DataHDDIOTensorBaseSPtr dst = boost::dynamic_pointer_cast<DataHDDIOTensorBase>( dstInfo.createDataHDDIO( false ) );
        LOG_INFO << "Copying U files..." << std::endl;
        for( byte u = 1; u <= 3; ++u )
            util::copyFile( src->getU123FileName(u), dst->getU123FileName(u) );

        LOG_INFO << "Reading original ranks..." << std::endl;
        constVolumeTreeBaseSPtr tree = src->getTree();

        LOG_INFO << "Converting ranks file..." << std::endl;
        const uint32_t ranksFileSize = util::fileSize( src->getRanksFileName() );
        const uint32_t iterationSize = tree->getSize()*sizeof(double);
        const uint32_t maxIterations = ranksFileSize / iterationSize;
        if( ranksFileSize != iterationSize*maxIterations )
            throw std::string( "Wrong size of ranks file: " ) + strUtil::toString( ranksFileSize );
        std::vector<double> ranksSrc( maxIterations*tree->getSize(), 0 );
        std::vector<double> ranksDst( maxIterations*tree->getSize(), 0 );

        if( !util::InFile::read( src->getRanksFileName(), std::ios::binary, 0, ranksFileSize, &ranksSrc[0] ))
            throw "Some error happen during reading of a file";

        // convert rank file
        double* rSrc = &ranksSrc[0];
        double* rDst = &ranksDst[0];
        LOG_INFO << "max iterations: " << maxIterations << std::endl;
        LOG_INFO << "iteration size: " << tree->getSize() << std::endl;
        for( uint32_t iter = 0; iter < maxIterations; ++iter )
        {
            for( size_t i = 0; i < tree->getSize(); ++i )
            {
                uint32_t nodeId = tree->getNodeId(i);
                if( nodeId > 0 )
                    rDst[i] = rSrc[ nodeId-1 ];
            }
            rSrc += tree->getSize();
            rDst += tree->getSize();
        }

        if( !util::OutFile::write( dst->getRanksFileName(), std::ios::binary | std::ios::trunc, 0, ranksFileSize, &ranksDst[0] ))
            throw "Some error happen during writing to a file.";


        // move cores to new locations
        LOG_INFO_ << "Allocating storage of " << tree->getNumberOfValidBlocks() <<
                     " blocks..." << std::endl;
        uint32_t validBlocks = tree->getNumberOfValidBlocks();
        if( !dst->allocateAllFiles( validBlocks ))
            throw "Can't allocate dst data files";
        LOG_INFO_ << "Copyig rank data." << std::endl;

        uint32_t counter       =  0;
        int64_t  oldPercents   = -1;
        uint32_t blocksPercentage = hlpFuncs::myClip( validBlocks/100u, 100u, 1000u );

        std::vector<byte> buffer( src->getMaxBlockSize() );
        uint32_t newId = 1;
        for( uint32_t i = 0; i < tree->getSize(); ++i )
        {
            uint32_t oldId = tree->getNodeId( i );
            if( oldId > 0 )
            {
                memset( &buffer[0], 0, buffer.size()*sizeof(buffer[0]) );
                src->read( oldId, &buffer[0] );

                if( !dst->write( newId++, &buffer[0] ))
                    throw "Can't write to a file ";

                // count how many blocks were already processed
                if( (++counter) % blocksPercentage == 0 )
                {
                    int64_t percents = int64_t(counter*100/validBlocks);

                    if( percents != oldPercents )
                    {
                        LOG_INFO << std::endl;
                        LOG_INFO << "copying blocks (" << counter << "/" << validBlocks << "): "
                                    << percents << "%" << std::flush;
                        oldPercents = percents;
                    }else
                        LOG_INFO << "." << std::flush;
                }
            }
        }
        LOG_INFO << std::endl;
        LOG_INFO_ << "Saving .dat file." << std::endl;
        dstInfo.save( dstArg.getValue( ) + "/data.dat" );
        LOG_INFO_ << "Done converting." << std::endl;
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

    return 0;
}


