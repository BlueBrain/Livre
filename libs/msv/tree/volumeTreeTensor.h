
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__VOLUME_TREE_TENSOR_H
#define MASS_VOL__VOLUME_TREE_TENSOR_H


#include <msv/tree/volumeTreeBase.h>

#include <msv/tree/rankErrors.h>


namespace massVolVis
{

class DataHDDIOTensorBase;

typedef std::vector<byte>  byteVec;
typedef std::vector<float> floatVec;

typedef boost::shared_ptr< const byteVec  > constbyteVecSPtr;
typedef boost::shared_ptr< const floatVec > constfloatVecSPtr;


class VolumeTreeTensor: public VolumeTreeBase
{
public:
    VolumeTreeTensor( const Vec3_ui16& srcSize, const uint32_t blockSize,
                      constNodeIdVecSPtr nodeIds,
                      constbyteVecSPtr ranks, constfloatVecSPtr scales );

    virtual ~VolumeTreeTensor(){}

    double getNodeScale( const uint32_t pos ) const;

    byte getNodeRank( const uint32_t pos ) const;

protected:
    constbyteVecSPtr  _ranks;  // ranks for different blocks (doesn't change over time)
    constfloatVecSPtr _scales; // tree itself (doesn't change over time)
};


typedef boost::shared_ptr< const RankErrorsVec > constRankErrorsVecSPtr;

class VolumeTreeTensorErrors: public VolumeTreeTensor
{
public:
    VolumeTreeTensorErrors( const Vec3_ui16& srcSize, const uint32_t blockSize,
                      constNodeIdVecSPtr nodeIds, constbyteVecSPtr ranks,
                      constfloatVecSPtr scales, constRankErrorsVecSPtr errors );

    virtual ~VolumeTreeTensorErrors(){}

    byte getValidRank(     uint32_t pos, uint16_t error ) const;
    bool isQualityTooLow(  uint32_t pos, uint16_t error ) const;
    bool isQualityTooHigh( uint32_t pos, uint16_t error ) const;

protected:
    constRankErrorsVecSPtr _errors;
};

} //namespace massVolVis


#endif // MASS_VOL__VOLUME_TREE_TENSOR_H

