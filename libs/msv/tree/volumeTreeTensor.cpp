
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include <msv/IO/dataHDDIOTensor.h>
#include <msv/tree/volumeTreeTensor.h>
#include <msv/util/debug.h>

#include <assert.h>

namespace massVolVis
{

VolumeTreeTensor::VolumeTreeTensor( const Vec3_ui16& srcSize, const uint32_t blockSize,
                                    constNodeIdVecSPtr nodeIds,
                                    constbyteVecSPtr ranks, constfloatVecSPtr scales )
    : VolumeTreeBase( srcSize, blockSize )
    , _ranks(  ranks )
    , _scales( scales )
{
    size_t treeSize = getSize();

    if( nodeIds && nodeIds->size() == treeSize )
        _replaceTree( nodeIds );

    if( !_ranks  || _ranks->size()  < treeSize )
        _ranks = constbyteVecSPtr(   new  byteVec( treeSize ));

    if( !_scales || _scales->size() < treeSize )
        _scales = constfloatVecSPtr( new floatVec( treeSize ));
}


double VolumeTreeTensor::getNodeScale( const uint32_t pos ) const
{
    if( pos >= _scales->size() )
    {
        LOG_ERROR << "Invalid pos: " << pos << std::endl;
        return 0.;
    }
    return (*_scales)[ pos ];
}


byte VolumeTreeTensor::getNodeRank( const uint32_t pos ) const
{
    if( pos >= _ranks->size() )
    {
        LOG_ERROR << "Invalid pos: " << pos << std::endl;
        return 0;
    }
    return (*_ranks)[ pos ];
}


VolumeTreeTensorErrors::VolumeTreeTensorErrors(
                      const Vec3_ui16& srcSize,const uint32_t blockSize,
                      constNodeIdVecSPtr nodeIds, constbyteVecSPtr ranks,
                      constfloatVecSPtr scales, constRankErrorsVecSPtr errors )
    : VolumeTreeTensor( srcSize, blockSize, nodeIds, ranks, scales )
    , _errors( errors )
{
    size_t treeSize = getSize();

    if( !_errors || _errors->size() < treeSize )
        _scales = constfloatVecSPtr( new floatVec( treeSize ));
}


namespace
{
bool _isPosValid( uint32_t pos, uint32_t size )
{
    if( pos < size )
        return true;

    LOG_ERROR << "Invalid pos: " << pos << std::endl;
    return false;
}
}


byte VolumeTreeTensorErrors::getValidRank( uint32_t pos, uint16_t error ) const
{
    return _isPosValid( pos, _errors->size()) ? (*_errors)[ pos ].getValidRank( error ) : 0;
}


bool VolumeTreeTensorErrors::isQualityTooLow(  uint32_t pos, uint16_t error ) const
{
    return _isPosValid( pos, _errors->size()) ? (*_errors)[ pos ].isQualityTooLow( error ) : 0;
}


bool VolumeTreeTensorErrors::isQualityTooHigh( uint32_t pos, uint16_t error ) const
{
    return _isPosValid( pos, _errors->size()) ? (*_errors)[ pos ].isQualityTooHigh( error ) : 0;
}

} //namespace massVolVis


